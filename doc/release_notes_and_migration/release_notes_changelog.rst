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

* Added the :option:`CONFIG_DESKTOP_CPU_LOAD_EVENT` Kconfig option.
  This option enables the :c:struct:`cpu_load_event`.
* Updated the :ref:`nrf_desktop_cpu_meas` to allow emitting the measured CPU load directly through the :ref:`nrf:nrf_profiler` API (:option:`CONFIG_DESKTOP_CPU_MEAS_RESULT_NRF_PROFILER`), which minimizes the impact on the measured CPU load.
  The option is enabled by default if the nRF Profiler (:kconfig:option:`CONFIG_NRF_PROFILER`) is enabled.

  Submitting CPU load as a :c:struct:`cpu_load_event` is controlled through the newly introduced :option:`CONFIG_DESKTOP_CPU_MEAS_RESULT_EVENT` Kconfig option.
  The feature is disabled by default if the :option:`CONFIG_DESKTOP_CPU_MEAS_RESULT_NRF_PROFILER` option is enabled.

Host tools
==========

|no_changes_yet_note|

Libraries and subsystems
========================

|no_changes_yet_note|
