.. _nrf_desktop_ram_pwrdn:

RAM power down module
#####################

.. contents::
   :local:
   :depth: 2

The RAM power down module gates unused SRAM after boot to reduce System ON leakage on supported nRF54L application cores.
The module is enabled by default on SoCs supported by :kconfig:option:`CONFIG_RAM_POWER_DOWN_LIBRARY`.

Module events
*************

.. include:: event_propagation.rst
    :start-after: table_ram_pwrdn_start
    :end-before: table_ram_pwrdn_end

.. note::
    |nrf_desktop_module_event_note|

Configuration
*************

To enable the module, use the :option:`CONFIG_DESKTOP_RAM_PWRDN` Kconfig option.
It is implied by :option:`CONFIG_DESKTOP_COMMON_MODULES` on supported SoCs.

The module selects :kconfig:option:`CONFIG_RAM_POWER_DOWN_LIBRARY`.

On RAM load builds, the module may gate ``cpuapp_sram_mcuboot_ram_region`` and ``cpuapp_sram_unused_ram_region`` when defined in the board :file:`memory_map_ram_load.dtsi`.
See :ref:`nrf_desktop_configuring_mcuboot_bootloader_ram_load` for the RAM layout requirements.

Implementation details
**********************

The module runs when :ref:`nrf_desktop_main` reports ``MODULE_STATE_READY`` (using :c:struct:`module_state_event`, same pattern as :ref:`nrf_desktop_watchdog`).
It then gates unused SRAM and reports ``MODULE_STATE_READY``.

Gating uses complete 32 KiB RAM sections on nRF54L application cores; partial ranges are not gated.
Within the chosen ``zephyr,sram`` node, only the unused tail above the linked application RAM end is considered.
RAM load maps may define additional regions (for example ``cpuapp_sram_unused_ram_region`` or the tail of ``cpuapp_sram_mcuboot_ram_region``) that are gated explicitly.
