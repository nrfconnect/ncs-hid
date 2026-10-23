.. _nrf_desktop_ble_state:
.. _nrf_desktop_ble_state_extension:

Bluetooth LE state module
#########################

.. contents::
   :local:
   :depth: 2

In nRF Desktop, the Bluetooth® LE state module is responsible for the following actions:

* Enabling Bluetooth (:c:func:`bt_enable`).
* Handling Zephyr connection callbacks (:c:struct:`bt_conn_cb`).
* Propagating information about the connection state and parameters by using :ref:`nrf:app_event_manager` events.

The module's functionality is additionally extended through the :ref:`CAF BLE state extension module <caf_ble_state_extension>`.
The documentation page contains information about both application modules.

Module events
*************

.. include:: event_propagation.rst
    :start-after: table_ble_state_start
    :end-before: table_ble_state_end

.. include:: event_propagation.rst
    :start-after: table_ble_state_extension_start
    :end-before: table_ble_state_extension_end

.. note::
    |nrf_desktop_module_event_note|

Configuration
*************

nRF Desktop uses an application-specific implementation of the Bluetooth LE state module.
Currently, the implementation is functionally identical to the :ref:`CAF Bluetooth LE state <nrf:caf_ble_state>` module, with the exception of the Kconfig naming.

.. note::
   The nRF Desktop documentation for this module will be ported in the future.
   Until then, see the :ref:`CAF Bluetooth LE state <nrf:caf_ble_state>` page for implementation details.

When Shorter Connection Intervals (SCI) are enabled, the |addon| also enables the :option:`CONFIG_CAF_BLE_STATE_EXTENSION` and :option:`CONFIG_CAF_BLE_SCI_CONN_RATE_EVENTS` options to provide the :c:struct:`ble_peer_sci_conn_rate_event` until it is available in the used |NCS| release.

.. note::
   Connection parameter changes behave very differently when HID SCI is used.
   The connection parameter update flow described on the :ref:`CAF Bluetooth LE state <nrf:caf_ble_state>` page does not apply.

See the :ref:`CAF BLE state extension module <caf_ble_state_extension>` page for implementation details about connection parameter changes when using SCI.

For more information about Bluetooth configuration in nRF Desktop, see :ref:`nrf_desktop_bluetooth_guide`.
