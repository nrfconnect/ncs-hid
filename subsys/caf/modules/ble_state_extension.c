/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/hci.h>

#include <caf/events/ble_common_event_extension.h>

#define MODULE ble_state_extension
#include <caf/events/module_state_event.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_CAF_BLE_STATE_EXTENSION_LOG_LEVEL);

#if CONFIG_CAF_BLE_SCI_CONN_RATE_EVENTS

static void conn_rate_changed(struct bt_conn *conn, uint8_t status,
			      const struct bt_conn_le_conn_rate_changed *params)
{
	struct ble_peer_sci_conn_rate_event *event = new_ble_peer_sci_conn_rate_event();

	event->id = conn;
	event->status = status;

	if (status == BT_HCI_ERR_SUCCESS) {
		__ASSERT(params != NULL,
			 "params pointer is NULL for successful conn rate change");

		event->params = *params;
	}

	APP_EVENT_SUBMIT(event);
}

#endif /* CONFIG_CAF_BLE_SCI_CONN_RATE_EVENTS */

static int register_conn_cbs(void)
{
	static struct bt_conn_cb conn_callbacks;

	if (IS_ENABLED(CONFIG_CAF_BLE_SCI_CONN_RATE_EVENTS)) {
		conn_callbacks.conn_rate_changed = conn_rate_changed;
	}

	bt_conn_cb_register(&conn_callbacks);

	return 0;
}

static bool app_event_handler(const struct app_event_header *aeh)
{
	if (is_module_state_event(aeh)) {
		const struct module_state_event *event =
			cast_module_state_event(aeh);

		if (check_state(event, MODULE_ID(main), MODULE_STATE_READY)) {
			static bool initialized;

			__ASSERT_NO_MSG(!initialized);
			initialized = true;

			register_conn_cbs();
		}

		return false;
	}

	__ASSERT_NO_MSG(false);

	return false;
}

APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);
