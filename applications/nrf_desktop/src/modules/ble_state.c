/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/types.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>

#include <caf/events/ble_common_event.h>
#ifdef CONFIG_DESKTOP_BLE_USE_LLPM
#include <bluetooth/hci_vs_sdc.h>
#endif /* CONFIG_DESKTOP_BLE_USE_LLPM */

#define MODULE ble_state
#include <caf/events/module_state_event.h>
#include <caf/events/module_suspend_event.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_DESKTOP_BLE_STATE_LOG_LEVEL);


struct bond_find_data {
	const bt_addr_le_t *peer_address;
	bool peer_bonded;
	uint8_t bond_cnt;
};

enum state {
	STATE_DISABLED,
	STATE_DISABLED_SUSPEND_PENDING,
	STATE_INITIALIZING,
	STATE_INITIALIZING_SUSPEND_PENDING,
	STATE_SETTINGS_LOADING,
	STATE_SETTINGS_LOADING_SUSPEND_PENDING,
	STATE_RESUMING,
	STATE_RESUMING_SUSPEND_PENDING,
	STATE_SUSPENDED,
	STATE_ACTIVE,
	STATE_ERROR,
};

static enum state state = STATE_DISABLED;

static void update_state(enum state new_state);

static void bond_check_cb(const struct bt_bond_info *info, void *user_data)
{
	struct bond_find_data *data = user_data;

	data->bond_cnt++;
	if (!bt_addr_le_cmp(&info->addr, data->peer_address)) {
		data->peer_bonded = true;
	}

	if (IS_ENABLED(CONFIG_LOG)) {
		char addr_str[BT_ADDR_LE_STR_LEN];

		bt_addr_le_to_str(&info->addr, addr_str, sizeof(addr_str));
		LOG_INF("Already bonded to %s", addr_str);
	}
}

static void disconnect_peer(struct bt_conn *conn)
{
	int err = bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);

	if (err && (err != -ENOTCONN)) {
		LOG_ERR("Failed to disconnect peer (err=%d)", err);
		update_state(STATE_ERROR);
	} else {
		LOG_INF("Peer disconnected");
	}
}

static void broadcast_init_conn_params(struct bt_conn *conn)
{
	struct bt_conn_info info;
	int err = bt_conn_get_info(conn, &info);

	if (err) {
		LOG_ERR("Cannot get conn info (%d)", err);
	} else {
		struct ble_peer_conn_params_event *event = new_ble_peer_conn_params_event();
		uint16_t info_interval_1250us = BT_GAP_US_TO_CONN_INTERVAL(info.le.interval_us);

		event->id = conn;
		event->interval_min = info_interval_1250us;
		event->interval_max = info_interval_1250us;
		event->latency = info.le.latency;
		event->timeout = info.le.timeout;
		event->updated = true;

		APP_EVENT_SUBMIT(event);
	}
}

static void connected(struct bt_conn *conn, uint8_t error)
{
	/* Make sure that connection will remain valid. */
	bt_conn_ref(conn);

	if (error) {
		struct ble_peer_event *event = new_ble_peer_event();

		event->id = conn;
		event->state = PEER_STATE_CONN_FAILED;
		event->reason = error;
		APP_EVENT_SUBMIT(event);

		if (IS_ENABLED(CONFIG_LOG)) {
			char addr_str[BT_ADDR_LE_STR_LEN];

			bt_addr_le_to_str(bt_conn_get_dst(conn), addr_str, sizeof(addr_str));
			LOG_WRN("Failed to connect to %s (%u)", addr_str, error);
		}

		return;
	}

	if (IS_ENABLED(CONFIG_LOG)) {
		char addr_str[BT_ADDR_LE_STR_LEN];

		bt_addr_le_to_str(bt_conn_get_dst(conn), addr_str, sizeof(addr_str));
		LOG_INF("Connected to %s", addr_str);
	}

	struct ble_peer_event *event = new_ble_peer_event();

	event->id = conn;
	event->state = PEER_STATE_CONNECTED;
	event->reason = 0;
	APP_EVENT_SUBMIT(event);

	broadcast_init_conn_params(conn);

	struct bt_conn_info info;

	int err = bt_conn_get_info(conn, &info);

	if (err) {
		LOG_WRN("Cannot get conn info");
		goto disconnect;
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    (info.role == BT_CONN_ROLE_PERIPHERAL)) {
		struct bond_find_data bond_find_data = {
			.peer_address = bt_conn_get_dst(conn),
			.peer_bonded = false,
			.bond_cnt = 0
		};

		bt_foreach_bond(info.id, bond_check_cb, &bond_find_data);

		LOG_INF("Identity %" PRIu8 " has %" PRIu8 " bonds",
			info.id, bond_find_data.bond_cnt);

		if (!bond_find_data.peer_bonded &&
		    (bond_find_data.bond_cnt >= CONFIG_DESKTOP_BLE_STATE_MAX_LOCAL_ID_BONDS)) {
			goto disconnect;
		}
	}

	if (IS_ENABLED(CONFIG_DESKTOP_BLE_STATE_SECURITY_REQ)) {
		/* Security must be enabled after peer event is sent.
		 * This is to make sure notification events are propagated
		 * in the right order.
		 */

		LOG_INF("Set security level");
		err = bt_conn_set_security(conn, BT_SECURITY_L2);
		if (err) {
			LOG_ERR("Failed to set security");
			goto disconnect;
		}
	}

	return;

disconnect:
	disconnect_peer(conn);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	if (IS_ENABLED(CONFIG_LOG)) {
		char addr_str[BT_ADDR_LE_STR_LEN];

		bt_addr_le_to_str(bt_conn_get_dst(conn), addr_str, sizeof(addr_str));
		LOG_INF("Disconnected from %s (reason %u)", addr_str, reason);
	}

	struct ble_peer_event *event = new_ble_peer_event();

	event->id = conn;
	event->state = PEER_STATE_DISCONNECTED;
	event->reason = reason;
	APP_EVENT_SUBMIT(event);
}

static struct bt_gatt_exchange_params exchange_params;

static void exchange_func(struct bt_conn *conn, uint8_t err,
			  struct bt_gatt_exchange_params *params)
{
	LOG_INF("MTU exchange done");
}

static void security_changed(struct bt_conn *conn, bt_security_t level,
			     enum bt_security_err bt_err)
{
	int err;

	if (IS_ENABLED(CONFIG_LOG)) {
		char addr_str[BT_ADDR_LE_STR_LEN];

		bt_addr_le_to_str(bt_conn_get_dst(conn), addr_str, sizeof(addr_str));

		if (!bt_err && (level >= BT_SECURITY_L2)) {
			LOG_INF("Security with %s level %u", addr_str, level);
		} else {
			LOG_WRN("Security with %s failed, level %u err %d",
				addr_str, level, bt_err);
		}
	}

	if (bt_err || (level < BT_SECURITY_L2)) {
		if (IS_ENABLED(CONFIG_DESKTOP_BLE_STATE_SECURITY_REQ)) {
			disconnect_peer(conn);
		}

		return;
	}

	struct ble_peer_event *event = new_ble_peer_event();

	event->id = conn;
	event->state = PEER_STATE_SECURED;
	event->reason = 0;
	APP_EVENT_SUBMIT(event);

	if (IS_ENABLED(CONFIG_DESKTOP_BLE_STATE_EXCHANGE_MTU)) {
		exchange_params.func = exchange_func;
		err = bt_gatt_exchange_mtu(conn, &exchange_params);
		if (err) {
			LOG_ERR("MTU exchange failed");
		}
	}
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	struct ble_peer_conn_params_event *event =
		new_ble_peer_conn_params_event();

	event->id = conn;
	event->interval_min = param->interval_min;
	event->interval_max = param->interval_max;
	event->latency = param->latency;
	event->timeout = param->timeout;
	event->updated = false;

	APP_EVENT_SUBMIT(event);

	return false;
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval,
			     uint16_t latency, uint16_t timeout)
{
	struct ble_peer_conn_params_event *event =
		new_ble_peer_conn_params_event();

	event->id = conn;
	event->interval_min = interval;
	event->interval_max = interval;
	event->latency = latency;
	event->timeout = timeout;
	event->updated = true;

	APP_EVENT_SUBMIT(event);
}

static const char *state2str(enum state s)
{
	switch (s) {
	case STATE_DISABLED:
		return "DISABLED";
	case STATE_DISABLED_SUSPEND_PENDING:
		return "DISABLED_SUSPEND_PENDING";
	case STATE_INITIALIZING:
		return "INITIALIZING";
	case STATE_INITIALIZING_SUSPEND_PENDING:
		return "INITIALIZING_SUSPEND_PENDING";
	case STATE_SETTINGS_LOADING:
		return "SETTINGS_LOADING";
	case STATE_SETTINGS_LOADING_SUSPEND_PENDING:
		return "SETTINGS_LOADING_SUSPEND_PENDING";
	case STATE_RESUMING:
		return "RESUMING";
	case STATE_RESUMING_SUSPEND_PENDING:
		return "RESUMING_SUSPEND_PENDING";
	case STATE_SUSPENDED:
		return "SUSPENDED";
	case STATE_ACTIVE:
		return "ACTIVE";
	case STATE_ERROR:
		return "ERROR";
	default:
		__ASSERT_NO_MSG(false);
		return "UNKNOWN";
	}
}

static void broadcast_module_state(enum state prev_state, enum state new_state)
{
	static enum module_state last_notified_state = MODULE_STATE_COUNT;
	enum module_state module_state = last_notified_state;

	switch (new_state) {
	case STATE_DISABLED:
	case STATE_DISABLED_SUSPEND_PENDING:
	case STATE_INITIALIZING:
	case STATE_INITIALIZING_SUSPEND_PENDING:
	case STATE_SETTINGS_LOADING:
	case STATE_SETTINGS_LOADING_SUSPEND_PENDING:
		/* Notify that the module is ready so that the settings loader
		 * module can proceed with its work.
		 */
		module_state = MODULE_STATE_READY;
		break;
	case STATE_RESUMING:
	case STATE_RESUMING_SUSPEND_PENDING:
		/* No module state broadcast needed. */
		break;

	case STATE_ACTIVE:
		module_state = MODULE_STATE_READY;
		break;

	case STATE_SUSPENDED:
		module_state = MODULE_STATE_SUSPENDED;
		break;

	case STATE_ERROR:
		module_state = MODULE_STATE_ERROR;
		break;

	default:
		__ASSERT_NO_MSG(false);
		break;
	}

	if (module_state != last_notified_state) {
		module_set_state(module_state);
		last_notified_state = module_state;
	}
}

static void bt_ready(int err)
{
	if (err) {
		LOG_ERR("Bluetooth initialization failed (err %d)", err);
		sys_reboot(SYS_REBOOT_WARM);
	}

	LOG_INF("Bluetooth initialized");

#ifdef CONFIG_DESKTOP_BLE_USE_LLPM
	sdc_hci_cmd_vs_llpm_mode_set_t cmd_enable;

	cmd_enable.enable = 1;

	err = hci_vs_sdc_llpm_mode_set(&cmd_enable);
	if (err) {
		LOG_ERR("Error enabling LLPM (err: %d)", err);
	} else {
		LOG_INF("LLPM enabled");
	}
#endif /* CONFIG_DESKTOP_BLE_USE_LLPM */

	switch (state) {

	case STATE_INITIALIZING:
		if (IS_ENABLED(CONFIG_CAF_SETTINGS_LOADER)) {
			update_state(STATE_SETTINGS_LOADING);
		} else {
			update_state(STATE_ACTIVE);
		}
		break;
	case STATE_INITIALIZING_SUSPEND_PENDING:
		if (IS_ENABLED(CONFIG_CAF_SETTINGS_LOADER)) {
			update_state(STATE_SETTINGS_LOADING_SUSPEND_PENDING);
		} else {
			update_state(STATE_ACTIVE);
			update_state(STATE_SUSPENDED);
		}
		break;
	case STATE_RESUMING:
		update_state(STATE_ACTIVE);
		break;
	case STATE_RESUMING_SUSPEND_PENDING:
		update_state(STATE_ACTIVE);
		update_state(STATE_SUSPENDED);
		break;
	default:
		__ASSERT_NO_MSG(false);
		break;
	}
}

static int apply_state_transition_actions(enum state prev_state, enum state new_state)
{
	int err = 0;

	switch (new_state) {
	case STATE_INITIALIZING:
		if (prev_state == STATE_DISABLED) {
			err = bt_enable(bt_ready);
			if (err) {
				LOG_ERR("Bluetooth enable failed (err: %d)", err);
			}
		} else if (prev_state == STATE_INITIALIZING_SUSPEND_PENDING) {
			/* Suspend requested before bt_ready. */
		} else {
			__ASSERT_NO_MSG(false);
		}
		break;
	case STATE_INITIALIZING_SUSPEND_PENDING:
		if (prev_state == STATE_DISABLED_SUSPEND_PENDING) {
			err = bt_enable(bt_ready);
			if (err) {
				LOG_ERR("Bluetooth enable failed (err: %d)", err);
			}
		} else if (prev_state == STATE_INITIALIZING) {
			/* Suspend cleared before bt_ready. */
		} else {
			__ASSERT_NO_MSG(false);
		}
		break;
	case STATE_RESUMING:
		if (prev_state == STATE_SUSPENDED) {
			err = bt_enable(bt_ready);
			if (err) {
				LOG_ERR("Bluetooth enable failed (err: %d)", err);
			}
		} else if (prev_state == STATE_RESUMING_SUSPEND_PENDING) {
			/* Suspend cleared before bt_ready; enable already requested. */
		} else {
			__ASSERT_NO_MSG(false);
		}
		break;

	case STATE_ACTIVE:
		/* Do not load settings on first initialization, as they are already loaded by
		 * the CAF Settings loader module.
		 */
		if ((prev_state == STATE_RESUMING) ||
		    (prev_state == STATE_RESUMING_SUSPEND_PENDING)) {
			/* Settings must be reloaded after Bluetooth is reenabled. */
			err = settings_load_subtree("bt");
			if (err) {
				LOG_ERR("Bluetooth settings load failed (err: %d)", err);
			}
		} else if ((prev_state == STATE_SETTINGS_LOADING) ||
			   (prev_state == STATE_SETTINGS_LOADING_SUSPEND_PENDING)) {
			/* First initialization after bt_ready. */
		} else if (!IS_ENABLED(CONFIG_CAF_SETTINGS_LOADER) &&
			   ((prev_state == STATE_INITIALIZING) ||
			    (prev_state == STATE_INITIALIZING_SUSPEND_PENDING))) {
			/* First initialization after bt_ready. */
		} else {
			__ASSERT_NO_MSG(false);
		}
		break;

	case STATE_SUSPENDED:
		if (prev_state == STATE_ACTIVE) {
			err = bt_disable();
			if (err) {
				LOG_ERR("Bluetooth disable failed (err: %d)", err);
			}
		} else {
			__ASSERT_NO_MSG(false);
		}
		break;

	case STATE_SETTINGS_LOADING:
		__ASSERT_NO_MSG((prev_state == STATE_INITIALIZING)
				|| prev_state == (STATE_SETTINGS_LOADING_SUSPEND_PENDING));
		break;

	case STATE_SETTINGS_LOADING_SUSPEND_PENDING:
		__ASSERT_NO_MSG((prev_state == STATE_INITIALIZING_SUSPEND_PENDING)
				|| (prev_state == STATE_SETTINGS_LOADING));
		break;

	case STATE_DISABLED:
		__ASSERT_NO_MSG(prev_state == STATE_DISABLED_SUSPEND_PENDING);
		break;

	case STATE_DISABLED_SUSPEND_PENDING:
		__ASSERT_NO_MSG(prev_state == STATE_DISABLED);
		break;

	case STATE_INITIALIZING_SUSPEND_PENDING:
		__ASSERT_NO_MSG(prev_state == STATE_INITIALIZING);
		break;

	case STATE_RESUMING_SUSPEND_PENDING:
		__ASSERT_NO_MSG(prev_state == STATE_RESUMING);
		break;

	case STATE_ERROR:
		break;

	default:
		__ASSERT_NO_MSG(false);
		break;
	}

	return err;
}

static void update_state(enum state new_state)
{
	if (state == STATE_ERROR) {
		return;
	}

	if (state == new_state) {
		return;
	}

	enum state prev_state = state;
	int err = apply_state_transition_actions(prev_state, new_state);

	if (err) {
		LOG_ERR("State transition failed (err: %d). Entering ERROR state.", err);
		state = STATE_ERROR;
	} else {
		state = new_state;
		LOG_DBG("State: %s", state2str(state));
	}

	broadcast_module_state(prev_state, state);
}

static bool handle_module_suspend_req_event(const struct module_suspend_req_event *event)
{
	if (event->sink_module_id != MODULE_ID(ble_state)) {
		/* Not us. */
		return false;
	}

	switch (state) {
	case STATE_DISABLED:
		update_state(STATE_DISABLED_SUSPEND_PENDING);
		break;
	case STATE_SETTINGS_LOADING:
		update_state(STATE_SETTINGS_LOADING_SUSPEND_PENDING);
		break;
	case STATE_INITIALIZING:
		update_state(STATE_INITIALIZING_SUSPEND_PENDING);
		break;
	case STATE_RESUMING:
		update_state(STATE_RESUMING_SUSPEND_PENDING);
		break;
	case STATE_ACTIVE:
		update_state(STATE_SUSPENDED);
		break;
	case STATE_DISABLED_SUSPEND_PENDING:
	case STATE_INITIALIZING_SUSPEND_PENDING:
	case STATE_SETTINGS_LOADING_SUSPEND_PENDING:
	case STATE_RESUMING_SUSPEND_PENDING:
	case STATE_SUSPENDED:
	case STATE_ERROR:
		break;
	default:
		__ASSERT_NO_MSG(false);
		break;
	}

	return false;
}

static bool handle_module_resume_req_event(const struct module_resume_req_event *event)
{
	if (event->sink_module_id != MODULE_ID(ble_state)) {
		/* Not us. */
		return false;
	}

	switch (state) {
	case STATE_DISABLED_SUSPEND_PENDING:
		update_state(STATE_DISABLED);
		break;
	case STATE_INITIALIZING_SUSPEND_PENDING:
		update_state(STATE_INITIALIZING);
		break;
	case STATE_SETTINGS_LOADING_SUSPEND_PENDING:
		update_state(STATE_SETTINGS_LOADING);
		break;
	case STATE_RESUMING_SUSPEND_PENDING:
	case STATE_SUSPENDED:
		update_state(STATE_RESUMING);
		break;
	case STATE_INITIALIZING:
	case STATE_DISABLED:
	case STATE_SETTINGS_LOADING:
	case STATE_RESUMING:
	case STATE_ACTIVE:
	case STATE_ERROR:
		break;
	default:
		__ASSERT_NO_MSG(false);
		break;
	}

	return false;
}

static void handle_settings_loaded(void)
{
	switch (state) {
	case STATE_SETTINGS_LOADING:
		update_state(STATE_ACTIVE);
		break;
	case STATE_SETTINGS_LOADING_SUSPEND_PENDING:
		update_state(STATE_ACTIVE);
		update_state(STATE_SUSPENDED);
		break;
	default:
		__ASSERT_NO_MSG(false);
	}
}

static void ble_state_init(void)
{
	static struct bt_conn_cb conn_callbacks = {
		.connected = connected,
		.disconnected = disconnected,
		.security_changed = security_changed,
		.le_param_req = le_param_req,
		.le_param_updated = le_param_updated,
	};
	bt_conn_cb_register(&conn_callbacks);

	switch (state) {
	case STATE_DISABLED:
		update_state(STATE_INITIALIZING);
		break;
	case STATE_DISABLED_SUSPEND_PENDING:
		update_state(STATE_INITIALIZING_SUSPEND_PENDING);
		break;
	default:
		__ASSERT_NO_MSG(false);
		break;
	}
}

static bool app_event_handler(const struct app_event_header *aeh)
{
	if (is_module_state_event(aeh)) {
		const struct module_state_event *event =
			cast_module_state_event(aeh);

		if (check_state(event, MODULE_ID(main), MODULE_STATE_READY)) {

			__ASSERT_NO_MSG((state == STATE_DISABLED)
					|| (state == STATE_DISABLED_SUSPEND_PENDING));
			ble_state_init();
		}

		if (IS_ENABLED(CONFIG_CAF_SETTINGS_LOADER) &&
		    check_state(event, MODULE_ID(settings_loader), MODULE_STATE_READY)) {
			handle_settings_loaded();
		}

		return false;
	}

	if (is_ble_peer_event(aeh)) {
		const struct ble_peer_event *event = cast_ble_peer_event(aeh);

		switch (event->state) {
		case PEER_STATE_CONN_FAILED:
		case PEER_STATE_DISCONNECTED:
			/* Connection object is no longer in use. */
			bt_conn_unref(event->id);
			break;

		default:
			/* Ignore. */
			break;
		}

		return false;
	}

	if (IS_ENABLED(CONFIG_DESKTOP_BLE_STATE_MODULE_SUSPEND_EVENTS) &&
	    is_module_suspend_req_event(aeh)) {
		return handle_module_suspend_req_event(cast_module_suspend_req_event(aeh));
	}

	if (IS_ENABLED(CONFIG_DESKTOP_BLE_STATE_MODULE_SUSPEND_EVENTS) &&
	    is_module_resume_req_event(aeh)) {
		return handle_module_resume_req_event(cast_module_resume_req_event(aeh));
	}

	/* If event is unhandled, unsubscribe. */
	__ASSERT_NO_MSG(false);

	return false;
}

APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);
APP_EVENT_SUBSCRIBE_FINAL(MODULE, ble_peer_event);
#ifdef CONFIG_DESKTOP_BLE_STATE_MODULE_SUSPEND_EVENTS
APP_EVENT_SUBSCRIBE(MODULE, module_suspend_req_event);
APP_EVENT_SUBSCRIBE(MODULE, module_resume_req_event);
#endif /* CONFIG_DESKTOP_BLE_STATE_MODULE_SUSPEND_EVENTS */
