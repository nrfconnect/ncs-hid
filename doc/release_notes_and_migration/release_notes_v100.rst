.. _release_notes_addon_v100:

Release notes for |addon| v1.0.0
################################

.. contents::
   :local:
   :depth: 2

The |addon| v1.0.0 is the first official release of the standalone HID Add-on for the |NCS|.
It delivers the nRF Desktop HID reference design application, host-side HID configurator tools, and dedicated documentation for developing Human Interface Devices on nRF54L Series SoCs.

This release is based on the |NCS| v3.4.1 release tag and toolchain.

Highlights
**********

* First standalone |NCS| add-on (``ncs-hid``) for HID development on top of the |NCS| v3.4.1.
* nRF Desktop application ported from sdk-nrf with a narrowed scope to support only nRF54L Series development kits.
* Experimental end-to-end HID Shorter Connection Intervals (HID SCI) support in nRF Desktop for both peripheral and dongle roles.

Release tag
***********

The release tag for the |addon| manifest repository (``ncs-hid``) is **v1.0.0**.

Known issues
************

For the list of issues valid for this release, navigate to `known issues page on the main branch`_ and select ``v1.0.0`` from the dropdown list.

Changelog
*********

This is the initial release of the |addon|.
The following sections list components ported from sdk-nrf v3.4.1 and describe additions and removals in the nRF Desktop application compared with that release.

Ported from sdk-nrf (|NCS| v3.4.1)
**********************************

The add-on depends on the full |NCS| v3.4.1 toolchain and libraries imported through west.
The following components were extracted from sdk-nrf at the |NCS| v3.4.1 release and are maintained in this add-on repository:

* nRF Desktop application - The complete :ref:`nrf_desktop` HID reference design at :file:`applications/nrf_desktop/`, including application source, board configurations, and application documentation.
* HID configurator scripts - The :ref:`nrf_desktop_config_channel_script` host tool at :file:`scripts/hid_configurator/`, used to configure nRF Desktop devices and perform firmware updates over the configuration channel.
* Documentation -  nRF Desktop and HID configurator documentation, plus add-on-specific pages for setup, glossary, known issues, and release notes.

Changes compared to sdk-nrf nRF Desktop (|NCS| v3.4.1)
******************************************************

The nRF Desktop application in this add-on is derived from sdk-nrf but differs in scope, platform support, and several functional areas.

Added
=====

* HID SCI support in nRF Desktop - Complete end-to-end HID Shorter Connection Intervals (HID SCI) integration for the nRF Desktop application, with additional fixes and integration work.
  This feature is experimental.
  Functionality and verification are incomplete and may change in future releases.

  * HID Shorter Connection Intervals (SCI) support on the dongle side.
    The :ref:`nrf_desktop_hid_forward` module uses :c:macro:`APP_EVENT_SUBSCRIBE_FIRST` to subscribe to the :c:struct:`ble_discovery_complete_event` event.
    The module updates event data to ensure all other modules are notified about the SCI support.
    The :ref:`nrf_desktop_ble_conn_params` module controls the connection parameters by requesting HID SCI modes from the peripheral and scales the minimum connection interval when multiple HID SCI peripherals are connected.
    Enable the feature with the :option:`CONFIG_DESKTOP_HID_FORWARD_HID_SCI_ENABLE` Kconfig option.
  * HID Shorter Connection Intervals (SCI) support on the peripheral side.
    The :ref:`nrf_desktop_hids` module enables support for the feature in the underlying HID GATT Service.
    The :ref:`nrf_desktop_ble_latency` module handles HID SCI mode change requests and the related connection parameter updates.
    Enable the feature with the :option:`CONFIG_DESKTOP_HIDS_SCI_ENABLE` Kconfig option.

  Supported build types:

  * Peripheral (mouse) - ``hid_sci`` and ``release_hid_sci`` build types on the nRF54L15 DK (:file:`nrf54l15dk/nrf54l05/cpuapp`, :file:`nrf54l15dk/nrf54l10/cpuapp`, and :file:`nrf54l15dk/nrf54l15/cpuapp`), nRF54LM20 DK (:file:`nrf54lm20dk/nrf54lm20a/cpuapp` and :file:`nrf54lm20dk/nrf54lm20b/cpuapp`), and nRF54LS05 DK (:file:`nrf54ls05dk/nrf54ls05a/cpuapp` and :file:`nrf54ls05dk/nrf54ls05b/cpuapp`).
  * Peripheral (keyboard) - ``hid_sci_keyboard`` and ``release_hid_sci_keyboard`` build types on the nRF54L15 DK (:file:`nrf54l15dk/nrf54l05/cpuapp`, :file:`nrf54l15dk/nrf54l10/cpuapp`, and :file:`nrf54l15dk/nrf54l15/cpuapp`) and nRF54LS05 DK (:file:`nrf54ls05dk/nrf54ls05a/cpuapp` and :file:`nrf54ls05dk/nrf54ls05b/cpuapp`).
  * Dongle - ``hid_sci_dongle`` and ``release_hid_sci_dongle`` build types on the nRF54LM20 DK (:file:`nrf54lm20dk/nrf54lm20a/cpuapp` and :file:`nrf54lm20dk/nrf54lm20b/cpuapp`).

* CAF Bluetooth LE SCI connection rate extension - The add-on ships a temporary Common Application Framework (CAF) extension at :file:`subsys/caf/` that provides the :c:struct:`ble_peer_sci_conn_rate_event` required by nRF Desktop HID SCI builds.
  The extension will be removed once the application event is available in an upcoming |NCS| release.
  See :ref:`caf_extensions`.

* nRF54LM20 DK LLPM dongle configurations - New ``dongle``, ``release_dongle``, ``dongle_4llpmconn``, and ``release_dongle_4llpmconn`` build types on the nRF54LM20 DK (:file:`nrf54lm20dk/nrf54lm20a/cpuapp` and :file:`nrf54lm20dk/nrf54lm20b/cpuapp`).

Removed
=======

* Board support - The support for the following platforms was removed from nRF Desktop:

  * nRF52 Series - ``nrf52820dongle_nrf52820``, ``nrf52833dk_nrf52820``, ``nrf52833dk_nrf52833``, ``nrf52833dongle_nrf52833``, ``nrf52840dk_nrf52840``, ``nrf52840dongle_nrf52840``, ``nrf52840gmouse_nrf52840``, ``nrf52dmouse_nrf52832``, ``nrf52kbd_nrf52832``
  * nRF53 Series - ``nrf5340dk_nrf5340_cpuapp``
  * nRF54H Series - ``nrf54h20dk_nrf54h20_cpuapp``

* DVFS module - Removed together with nRF54H20 support (:file:`src/modules/dvfs.c` together with related Kconfig and documentation).
* Partition Manager - All Partition Manager references removed.
  Memory layout is defined exclusively in devicetree.
  See :ref:`nrf_desktop_memory_layout`.
* Deprecated HID event queue Kconfig options - ``CONFIG_DESKTOP_HID_REPORT_EXPIRATION`` and ``CONFIG_DESKTOP_HID_EVENT_QUEUE_SIZE`` were removed from application configurations.

nRF Desktop performance measurements
************************************

The following tables summarize power consumption and HID report rate for the nRF Desktop application in this release.
Measurements use ``release``, ``release_hid_sci``, and ``release_ram_load`` build types on nRF54L Series development kits.

Power consumption
=================

Average current measured with a Power Profiler Kit II (PPK2) on selected nRF54L Series DKs.

.. list-table::
   :header-rows: 1

   * - Build type
     - Supply voltage
     - Measurement state
     - ``nrf54l15dk/nrf54l15/cpuapp``
     - ``nrf54lc10dk/nrf54lc10a/cpuapp``
     - ``nrf54lm20dk/nrf54lm20b/cpuapp``
     - ``nrf54ls05dk/nrf54ls05b/cpuapp``
   * - ``release_hid_sci``
     - 1.8 V
     - hid_sci_active
     - 0.021 mA
     - —
     - 0.028 mA
     - 0.023 mA
   * - ``release_hid_sci``
     - 1.8 V
     - hid_sci_motion_simulated
     - 4.51 mA
     - —
     - 4.75 mA
     - 3.58 mA
   * - ``release_hid_sci``
     - 1.8 V
     - hid_sci_suspended
     - 0.0095 mA
     - —
     - 0.015 mA
     - 0.012 mA
   * - ``release_ram_load``
     - 1.8 V
     - ble_active
     - —
     - —
     - 0.021 mA
     - —
   * - ``release_ram_load``
     - 1.8 V
     - ble_motion_simulated
     - —
     - —
     - 0.62 mA
     - —
   * - ``release_ram_load``
     - 1.8 V
     - ble_suspended
     - —
     - —
     - 0.02 mA
     - —
   * - ``release_ram_load``
     - 1.8 V
     - fast_advertising
     - —
     - —
     - 0.29 mA
     - —
   * - ``release_ram_load``
     - 1.8 V
     - llpm_active
     - —
     - —
     - 2.04 mA
     - —
   * - ``release_ram_load``
     - 1.8 V
     - llpm_motion_simulated
     - —
     - —
     - 3.14 mA
     - —
   * - ``release_ram_load``
     - 1.8 V
     - llpm_suspended
     - —
     - —
     - 0.049 mA
     - —
   * - ``release_ram_load``
     - 1.8 V
     - slow_advertising
     - —
     - —
     - 0.11 mA
     - —
   * - ``release_ram_load``
     - 1.8 V
     - system_off
     - —
     - —
     - 0.00046 mA
     - —
   * - ``release_ram_load``
     - 1.8 V
     - usb_mouse_active
     - —
     - —
     - 6.12 mA
     - —
   * - ``release_ram_load``
     - 1.8 V
     - usb_mouse_simulated
     - —
     - —
     - 6.12 mA
     - —
   * - ``release``
     - 1.8 V
     - ble_active
     - 0.015 mA
     - 0.016 mA
     - 0.021 mA
     - 0.017 mA
   * - ``release``
     - 1.8 V
     - ble_motion_simulated
     - 0.67 mA
     - 0.66 mA
     - 0.71 mA
     - 0.58 mA
   * - ``release``
     - 1.8 V
     - ble_suspended
     - 0.015 mA
     - 0.016 mA
     - 0.021 mA
     - 0.017 mA
   * - ``release``
     - 1.8 V
     - fast_advertising
     - 0.28 mA
     - 0.29 mA
     - 0.3 mA
     - 0.27 mA
   * - ``release``
     - 1.8 V
     - llpm_active
     - 2.15 mA
     - 2.19 mA
     - 2.25 mA
     - 1.81 mA
   * - ``release``
     - 1.8 V
     - llpm_motion_simulated
     - 3.69 mA
     - 3.61 mA
     - 3.84 mA
     - 2.91 mA
   * - ``release``
     - 1.8 V
     - llpm_suspended
     - 0.043 mA
     - 0.045 mA
     - 0.051 mA
     - 0.041 mA
   * - ``release``
     - 1.8 V
     - slow_advertising
     - 0.1 mA
     - 0.1 mA
     - 0.11 mA
     - 0.094 mA
   * - ``release``
     - 1.8 V
     - system_off
     - 0.001 mA
     - 0.0011 mA
     - 0.00047 mA
     - 0.0013 mA
   * - ``release``
     - 1.8 V
     - usb_mouse_active
     - —
     - —
     - 6.2 mA
     - —
   * - ``release``
     - 1.8 V
     - usb_mouse_simulated
     - —
     - —
     - 6.28 mA
     - —

HID report rate
===============

Effective HID input report rate measured on a connected HID host.

.. list-table::
   :header-rows: 1

   * - Build type
     - Transport
     - ``nrf54l15dk/nrf54l15/cpuapp``
     - ``nrf54lc10dk/nrf54lc10a/cpuapp``
     - ``nrf54lm20dk/nrf54lm20b/cpuapp``
     - ``nrf54ls05dk/nrf54ls05b/cpuapp``
   * - ``release_hid_sci``
     - hid_sci
     - 1330 Hz
     - —
     - 1333 Hz
     - 1333 Hz
   * - ``release_ram_load``
     - ble
     - —
     - —
     - 133 Hz
     - —
   * - ``release_ram_load``
     - llpm
     - —
     - —
     - 1000 Hz
     - —
   * - ``release_ram_load``
     - usb
     - —
     - —
     - 8000 Hz
     - —
   * - ``release``
     - ble
     - 133 Hz
     - 133 Hz
     - 133 Hz
     - 133 Hz
   * - ``release``
     - llpm
     - 999 Hz
     - 998 Hz
     - 1000 Hz
     - 1000 Hz
   * - ``release``
     - usb
     - —
     - —
     - 4032 Hz
     - —
