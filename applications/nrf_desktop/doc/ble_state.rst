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

The functionality of the module is extended through the :ref:`CAF BLE state extension module <caf_ble_state_extension>`.
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

nRF Desktop uses either the :ref:`CAF Bluetooth LE state <nrf:caf_ble_state>` module or an application-specific implementation of the Bluetooth LE state module.
To enable the application-specific implementation, set the :option:`CONFIG_DESKTOP_BLE_STATE_APP_SPECIFIC` Kconfig option.
The application-specific implementation is very similar to the :ref:`CAF Bluetooth LE state <nrf:caf_ble_state>` module, but uses nRF Desktop Kconfig option names and adds optional runtime suspend and resume support (see :ref:`nrf_desktop_ble_state_suspend`).

The following Kconfig options are additionally available for this module:

* :option:`CONFIG_DESKTOP_BLE_STATE_MODULE_SUSPEND_EVENTS` - This option enables support for module suspend and resume request events.
  It is available only for nRF Desktop Bluetooth peripheral devices (:option:`CONFIG_DESKTOP_BT_PERIPHERAL`) and enabled by default if prerequisites are met.
* :option:`CONFIG_DESKTOP_BLE_STATE_FREE_HW_RESOURCES_ON_SUSPEND` - This option frees allocated hardware resources (managed by the Multiprotocol Service Layer (MPSL)) when the module is suspended to allow radio reuse.
  It is available only with the SoftDevice Link Layer (:kconfig:option:`CONFIG_BT_LL_SOFTDEVICE`).

.. note::
   When using the application-specific Bluetooth LE state module (:option:`CONFIG_DESKTOP_BLE_STATE_APP_SPECIFIC`), Bluetooth is enabled during initial bring-up as soon as the module starts (after :c:func:`bt_enable` completes).
   A suspend request that arrives during that startup is held until startup finishes and the module can safely call :c:func:`bt_disable`.
   With :option:`CONFIG_CAF_SETTINGS_LOADER`, startup also covers loading persistent settings via the :ref:`CAF settings loader <nrf:caf_settings_loader>`.

Implementation details
**********************

.. _nrf_desktop_ble_state_suspend:

Suspending the module
=====================

.. note::
   Suspend and resume is currently supported only for nRF Desktop peripheral configurations.
   This is because the modules used by nRF Desktop dongle configurations do not handle Bluetooth LE state module suspend/resume request events.

When the :option:`CONFIG_DESKTOP_BLE_STATE_MODULE_SUSPEND_EVENTS` Kconfig option is enabled, you can suspend the module using the module suspend request event (:c:struct:`module_suspend_req_event`) directed to this module.
When entering the suspended state, the module disables Bluetooth (:c:func:`bt_disable`).
You can resume the module using the module resume request event (:c:struct:`module_resume_req_event`) directed to this module.
When resumed, the module re-enables Bluetooth (:c:func:`bt_enable`) and reloads Bluetooth LE settings.

A suspend request that arrives during Bluetooth LE state module initial bring-up is held until startup finishes and the module can safely call :c:func:`bt_disable`.
With :option:`CONFIG_CAF_SETTINGS_LOADER`, startup also covers loading persistent settings via the :ref:`CAF settings loader <nrf:caf_settings_loader>`.


Set the :option:`CONFIG_DESKTOP_BLE_STATE_FREE_HW_RESOURCES_ON_SUSPEND` option to free allocated hardware resources (managed by the Multiprotocol Service Layer (MPSL)) when the module is suspended.
This allows reusing the radio to communicate over another wireless protocol.
The option is available only with the SoftDevice Link Layer (:kconfig:option:`CONFIG_BT_LL_SOFTDEVICE`).

When Shorter Connection Intervals (SCI) are enabled, the |addon| also enables the :option:`CONFIG_CAF_BLE_STATE_EXTENSION` and :option:`CONFIG_CAF_BLE_SCI_CONN_RATE_EVENTS` options to provide the :c:struct:`ble_peer_sci_conn_rate_event` until it is available in the used |NCS| release.

.. note::
   Connection parameter changes behave very differently when HID SCI is used.
   The connection parameter update flow described on the :ref:`CAF Bluetooth LE state <nrf:caf_ble_state>` page does not apply.

See the :ref:`CAF BLE state extension module <caf_ble_state_extension>` page for implementation details about connection parameter changes when using SCI.

For more information about Bluetooth configuration in nRF Desktop, see :ref:`nrf_desktop_bluetooth_guide`.
