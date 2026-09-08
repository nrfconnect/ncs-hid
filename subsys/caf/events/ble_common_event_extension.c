/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdio.h>

#include <caf/events/ble_common_event_extension.h>

#if CONFIG_CAF_BLE_SCI_CONN_RATE_EVENTS

#include <zephyr/bluetooth/hci.h>

static void log_ble_peer_sci_conn_rate_event(const struct app_event_header *aeh)
{
	const struct ble_peer_sci_conn_rate_event *event =
		cast_ble_peer_sci_conn_rate_event(aeh);

	if (event->status == BT_HCI_ERR_SUCCESS) {
		APP_EVENT_MANAGER_LOG(aeh, "peer=%p interval_us=%" PRIu32
				      " subrate=%" PRIu16 " lat=%" PRIu16
				      " continuation num=%" PRIu16 " timeout=%" PRIu16,
				      event->id, event->params.interval_us,
				      event->params.subrate_factor,
				      event->params.peripheral_latency,
				      event->params.continuation_number,
				      event->params.supervision_timeout_10ms);
	} else {
		APP_EVENT_MANAGER_LOG(aeh, "peer=%p status=0x%02x", event->id,
				      event->status);
	}
}

APP_EVENT_TYPE_DEFINE(ble_peer_sci_conn_rate_event,
		      log_ble_peer_sci_conn_rate_event,
		      NULL,
		      APP_EVENT_FLAGS_CREATE(
			IF_ENABLED(CONFIG_CAF_INIT_LOG_BLE_SCI_CONN_RATE_EVENTS,
				(APP_EVENT_TYPE_FLAGS_INIT_LOG_ENABLE))));

#endif /* CONFIG_CAF_BLE_SCI_CONN_RATE_EVENTS */
