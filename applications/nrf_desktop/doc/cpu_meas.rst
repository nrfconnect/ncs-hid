.. _nrf_desktop_cpu_meas:

CPU measurement module
######################

.. contents::
   :local:
   :depth: 2

Use the CPU measurement module to monitor CPU load.

Module events
*************

.. include:: event_propagation.rst
    :start-after: table_cpu_meas_start
    :end-before: table_cpu_meas_end

.. note::
    |nrf_desktop_module_event_note|

Configuration
*************

To enable this module, use the :option:`CONFIG_DESKTOP_CPU_MEAS_ENABLE` Kconfig option.
This option selects the :kconfig:option:`CONFIG_NRF_CPU_LOAD` Kconfig option, which enables the :ref:`nrf:cpu_load` library that is used to perform the measurements.

Set the time between subsequent CPU load measurements, in milliseconds, using the :option:`CONFIG_DESKTOP_CPU_MEAS_PERIOD` option.

Measurement results
===================

The module can provide the measurement results in the following ways:

* :option:`CONFIG_DESKTOP_CPU_MEAS_RESULT_NRF_PROFILER` - The module uses the :ref:`nrf:nrf_profiler` API directly to emit the CPU load.
  This minimizes the impact of providing the results on the measured CPU load.
  The option depends on the :kconfig:option:`CONFIG_NRF_PROFILER` Kconfig option and is enabled by default if the nRF Profiler is enabled.
* :option:`CONFIG_DESKTOP_CPU_MEAS_RESULT_EVENT` - The module submits the CPU load as a :c:struct:`cpu_load_event`.
  The option selects the :option:`CONFIG_DESKTOP_CPU_LOAD_EVENT` Kconfig option, which enables the event.
  The option is enabled by default unless the :option:`CONFIG_DESKTOP_CPU_MEAS_RESULT_NRF_PROFILER` option is enabled.

You must enable at least one of these options.
You can enable both options at the same time.

Implementation details
**********************

The module periodically reads the CPU load, provides the result, and resets the measurement.
If reading the CPU load fails, the module reports an error state and stops the measurements.

The CPU load is presented in 0.001% units.
Depending on the configuration, the result is provided in the following ways:

* Through the nRF Profiler event - The module registers the ``cpu_load`` nRF Profiler event type with a single ``load`` field.
  The event is emitted only if profiling of the event is enabled.
* Through the application event - The :c:member:`cpu_load_event.load` presents the CPU load.
  The event can be displayed in the logs or using the :ref:`nrf:nrf_profiler`.
