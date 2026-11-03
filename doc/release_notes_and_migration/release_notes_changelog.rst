.. _release_notes_changelog:

Changelog for |hid_addon| v1.0.99
#################################

.. contents::
   :local:
   :depth: 2

The most relevant changes that are present on the main branch of the |hid_addon|, as compared to the latest official release, are tracked in this file.

.. note::
   This file is a work in progress and might not cover all relevant changes.

.. HOWTO

   When adding a new change, decide whether it needs an entry in the changelog.
   If it does, update this page.
   Add the sections you need, as only a handful of sections are kept when the changelog is cleaned for the next release.

Known issues
************

For the list of issues valid for this release, navigate to `known issues page on the main branch`_ and select the latest official tag from the dropdown list.

Changelog
*********

The following sections provide detailed lists of changes by component.

Applications
============

nRF Desktop
-----------

* Added:

  * An experimental application-specific :ref:`nrf_desktop_ble_state` implementation.
    The module ports the :ref:`CAF Bluetooth LE state <nrf:caf_ble_state>` implementation into the nRF Desktop so that Bluetooth LE state handling can be maintained as part of the application and extended with application-specific features in the future.
    nRF Desktop still uses the CAF module by default.
    Enable the :option:`CONFIG_DESKTOP_BLE_STATE_APP_SPECIFIC` Kconfig option to build with the application-specific implementation instead.
  * The :option:`CONFIG_DESKTOP_CPU_LOAD_EVENT` Kconfig option.
    This option enables the :c:struct:`cpu_load_event`.

* Updated the :ref:`nrf_desktop_cpu_meas` to allow emitting the measured CPU load directly through the :ref:`nrf:nrf_profiler` API (:option:`CONFIG_DESKTOP_CPU_MEAS_RESULT_NRF_PROFILER`), which minimizes the impact on the measured CPU load.
  The option is enabled by default if the nRF Profiler (:kconfig:option:`CONFIG_NRF_PROFILER`) is enabled.

  Submitting CPU load as a :c:struct:`cpu_load_event` is controlled through the newly introduced :option:`CONFIG_DESKTOP_CPU_MEAS_RESULT_EVENT` Kconfig option.
  The feature is disabled by default if the :option:`CONFIG_DESKTOP_CPU_MEAS_RESULT_NRF_PROFILER` option is enabled.

* Removed the deprecated ``hfclk_lock`` module and the ``CONFIG_DESKTOP_HFCLK_LOCK_ENABLE`` Kconfig option.
  For Low Latency Packet Mode (LLPM) configurations, use the :option:`CONFIG_DESKTOP_BLE_LOW_LATENCY_LOCK` Kconfig option instead.
  For HID Shorter Connection Intervals (HID SCI), you can achieve a similar effect by disabling connection subrating and setting the maximum peripheral latency to ``0`` for the active HID SCI mode (see the ``CONFIG_BT_HIDS_SCI_*_MAX_LATENCY`` Kconfig options and :ref:`nrf_desktop_ble_latency`).
  A concrete Twister example that applies these settings for the FAST mode is the ``ncs_hid.applications.nrf_desktop.zrelease_hid_sci_mouse_click`` scenario in the :file:`applications/nrf_desktop/sample.yaml` file.

Host tools
==========

|no_changes_yet_note|

Libraries and subsystems
========================

Common Application Framework (CAF) extensions
---------------------------------------------

* Added:

  * The :ref:`caf_buttons_isr` module (:option:`CONFIG_CAF_BUTTONS_ISR`), an alternative to :ref:`nrf:caf_buttons` that emits a :c:struct:`button_event` with minimal latency using GPIO interrupts.
    The module supports directly connected buttons, optional power management integration, and an optional dedicated workqueue for debouncing (:option:`CONFIG_CAF_BUTTONS_WQ`).
  * The :option:`CONFIG_CAF_BUTTONS_GPIO_SPURIOUS_ISR_WORKAROUND` Kconfig option to the :ref:`caf_buttons_isr` module.
    The option is available on nRF54LS05 SoCs and mitigates spurious button presses caused by GPIOTE PORT callbacks on shared GPIO ports (known issue ``NCSDK-41091``).
