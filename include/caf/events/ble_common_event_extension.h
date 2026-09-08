/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef _BLE_COMMON_EVENT_EXTENSION_H_
#define _BLE_COMMON_EVENT_EXTENSION_H_

/**
 * @file
 * @defgroup caf_ble_common_event_extension CAF Bluetooth LE common event extension
 * @{
 *
 * HID add-on extensions for @ref caf_ble_common_event that are
 * not yet available in the used nRF Connect SDK release.
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>

#include <app_event_manager.h>
#include <app_event_manager_profiler_tracer.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Bluetooth LE peer SCI connection rate event.
 *
 * The Bluetooth LE peer SCI connection rate event is submitted to inform that
 * connection rate parameters (including both connection interval and subrating)
 * have changed or that a connection rate change request failed.
 * The params field is valid only if status is BT_HCI_ERR_SUCCESS and must be
 * ignored otherwise.
 */
struct ble_peer_sci_conn_rate_event {
	/** Event header. */
	struct app_event_header header;

	/** ID used to identify Bluetooth connection - pointer to the bt_conn. */
	void *id;

	/** HCI Status from LE Connection Rate Change event. */
	uint8_t status;

	/** New connection rate parameters. Valid only if status is BT_HCI_ERR_SUCCESS. */
	struct bt_conn_le_conn_rate_changed params;
};

APP_EVENT_TYPE_DECLARE(ble_peer_sci_conn_rate_event);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* _BLE_COMMON_EVENT_EXTENSION_H_ */
