#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_library()

zephyr_library_sources_ifdef(CONFIG_CAF_BLE_STATE_EXTENSION
  modules/ble_state_extension.c
)

zephyr_library_sources_ifdef(CONFIG_CAF_BLE_COMMON_EVENTS_EXTENSION
  events/ble_common_event_extension.c
)
