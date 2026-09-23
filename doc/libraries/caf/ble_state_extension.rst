.. _caf_ble_state_extension:

CAF: Bluetooth LE state extension module
########################################

.. contents::
   :local:
   :depth: 2

The Bluetooth® LE state extension module extends the :ref:`CAF Bluetooth LE state module <nrf:caf_ble_state>` with functionality that is not yet available in the used |NCS| release.

Configuration
*************

The module is enabled by the :kconfig:option:`CONFIG_CAF_BLE_STATE_EXTENSION` Kconfig option.
The option depends on :kconfig:option:`CONFIG_CAF_BLE_STATE` and is enabled by default if prerequisites are met.

When Shorter Connection Intervals (SCI) is used, the :kconfig:option:`CONFIG_CAF_BLE_SCI_CONN_RATE_EVENTS` option is also required.
The option is enabled by default if prerequisites are met.

See the :ref:`caf_extensions_kconfig` page for the complete list of CAF extension Kconfig options.

|config|

For the API reference, see :ref:`CAF Bluetooth LE common event extension <nrf_desktop_caf_ble_common_event_extension>`.

Implementation details
**********************

The module is used by both Bluetooth Peripheral and Bluetooth Central devices.

It extends the :ref:`CAF Bluetooth LE state module <nrf:caf_ble_state>` by adding additional :ref:`nrf:app_event_manager` events.

Connection parameter change
===========================

Connection parameter changes are handled differently depending on whether Shorter Connection Intervals (SCI) are used for a given Bluetooth LE connection.
SCI support is controlled by the :kconfig:option:`CONFIG_BT_SHORTER_CONNECTION_INTERVALS` Kconfig option.
The peer must also support SCI for it to be used on a connection.

Shorter Connection Intervals unused
-----------------------------------

The |ble_state| handles Zephyr's callbacks for Bluetooth LE connections.
The module submits a :c:struct:`ble_peer_conn_params_event` to inform other application modules about connection parameter update requests and connection parameter updates.

The |ble_state| rejects the connection parameter update request in Zephyr's callback and submits the :c:struct:`ble_peer_conn_params_event` to inform other application modules about connection parameter update request.
The event is handled by the :ref:`nrf_desktop_ble_conn_params`, which updates the connection parameters.

Shorter Connection Intervals used
---------------------------------

If you are using SCI, update the connection parameters with the :c:func:`bt_conn_le_conn_rate_request` function.
The CAF Bluetooth LE state extension module submits a :c:struct:`ble_peer_sci_conn_rate_event` to inform other application modules about connection rate changes.
The event is also emitted when a connection rate update request fails.
In this case, it contains the status code of the failed request.
No event is emitted when a connection rate update request is received, because the Bluetooth stack does not provide a callback for this when SCI is enabled.

A Bluetooth LE Central can use the :c:func:`bt_conn_le_conn_rate_set_defaults` function to limit the accepted connection rate range.

.. note::
   When the :kconfig:option:`CONFIG_BT_SHORTER_CONNECTION_INTERVALS` Kconfig option is enabled, the non-SCI callbacks remain available.
   They are called if you use the non-SCI (connection parameter update) API to update the connection parameters.
   However, do not use the non-SCI API if both the device and peer support SCI.
