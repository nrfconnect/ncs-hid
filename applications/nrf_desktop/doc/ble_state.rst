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

For more information about the |ble_state| implementation, see the :ref:`CAF Bluetooth LE state <nrf:caf_ble_state>` page.

When Shorter Connection Intervals (SCI) are enabled, the |addon| also enables the :kconfig:option:`CONFIG_CAF_BLE_STATE_EXTENSION` and :kconfig:option:`CONFIG_CAF_BLE_SCI_CONN_RATE_EVENTS` options to provide the :c:struct:`ble_peer_sci_conn_rate_event` until it is available in the used |NCS| release.

.. note::
   Connection parameter changes behave very differently when HID SCI is used.
   The connection parameter update flow described on the :ref:`CAF Bluetooth LE state <nrf:caf_ble_state>` page does not apply.

See the :ref:`CAF BLE state extension module <caf_ble_state_extension>` page for implementation details about connection parameter changes when using SCI.

For more information about Bluetooth configuration in nRF Desktop, see :ref:`nrf_desktop_bluetooth_guide`.
