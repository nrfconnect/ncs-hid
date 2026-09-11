.. _nrf_desktop_ble_state:

Bluetooth LE state module
#########################

.. contents::
   :local:
   :depth: 2

In nRF Desktop, the Bluetooth® LE state module is responsible for the following actions:

* Enabling Bluetooth (:c:func:`bt_enable`).
* Handling Zephyr connection callbacks (:c:struct:`bt_conn_cb`).
* Propagating information about the connection state and parameters by using :ref:`app_event_manager` events.

Module events
*************

.. include:: event_propagation.rst
    :start-after: table_ble_state_start
    :end-before: table_ble_state_end

.. note::
    |nrf_desktop_module_event_note|

Configuration
*************

nRF Desktop uses the Bluetooth LE state module from :ref:`lib_caf` (CAF).
The :option:`CONFIG_DESKTOP_BLE_STATE` Kconfig option selects the :kconfig:option:`CONFIG_CAF_BLE_STATE` option.

For more information about the |ble_state| implementation, see the :ref:`CAF Bluetooth LE state <caf_ble_state>` page.

When Shorter Connection Intervals (SCI) are enabled, the |addon| also enables the :kconfig:option:`CONFIG_CAF_BLE_STATE_EXTENSION` and :kconfig:option:`CONFIG_CAF_BLE_SCI_CONN_RATE_EVENTS` options to provide the :c:struct:`ble_peer_sci_conn_rate_event` until it is available in the used |NCS| release.

For more information about Bluetooth configuration in nRF Desktop, see :ref:`nrf_desktop_bluetooth_guide`.

Implementation details
**********************

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

If you are using SCI, update the connection parameters with the connection *rate* API (:c:func:`bt_conn_le_conn_rate_request`).
The |ble_state| extension module submits a :c:struct:`ble_peer_sci_conn_rate_event` to inform other application modules about connection rate changes.
The event is also emitted when a connection rate update request fails.
In this case, it contains the status code of the failed request.
No event is emitted when a connection rate update request is received, because the Bluetooth stack does not provide a callback for this when SCI is enabled.

A Bluetooth LE Central can use the :c:func:`bt_conn_le_conn_rate_set_defaults` function to limit the accepted connection rate range.

.. note::
   When the :kconfig:option:`CONFIG_BT_SHORTER_CONNECTION_INTERVALS` Kconfig option is enabled, the non-SCI callbacks remain available.
   They are called if you use the non-SCI (connection parameter update) API to update the connection parameters.
   However, do not use the non-SCI API if both the device and peer support SCI.
