.. _hid_solution_overview:

Solution overview
#################

.. contents::
   :local:
   :depth: 2

The |addon| provides a complete solution for developing a :term:`Human Interface Device (HID)` on Nordic Semiconductor SoCs.
It combines device-side firmware, based on the nRF Desktop reference design, with host-side tools that configure and update the device at runtime.

This page describes the parts of the solution and how they work together.
For the requirements that you need to meet before you start, see the :ref:`hid_setup` page.

Solution components
*******************

The |addon| consists of the following components:

nRF Desktop application
   The :ref:`nrf_desktop` HID reference design application, which is the firmware that runs on the device.
   A single code base covers all supported device roles, and the role is selected through the application configuration.

HID configurator scripts
   The :ref:`nrf_desktop_config_channel_script` host tool, which is a set of Python scripts that run on a PC.
   The scripts discover connected nRF Desktop devices, read and write their runtime options, and perform firmware updates.

Both components rely on the |NCS| and are versioned together with the add-on.

Device roles
************

Depending on its configuration, the nRF Desktop application acts in one of the following roles:

Mouse
   A HID peripheral that reports motion, wheel, and button data.
   The application supports both desktop mouse and gaming mouse configurations.
   The gaming mouse configuration enables both the Bluetooth® LE and USB transports, so that the device can switch between wireless and wired operation.

Keyboard
   A HID peripheral that reports key presses from a button matrix, together with modifier keys and consumer control keys.

Dongle
   A USB device that acts as a Bluetooth LE central.
   It connects to HID peripherals over Bluetooth LE and retransmits their reports to the host over USB.
   The dongle lets you use HID peripherals with hosts that have no Bluetooth support, or that require a lower latency than the host Bluetooth stack provides.

Not all roles are available on every board.
For the roles supported by each board, see the :ref:`hid_setup` page, and for the details of each board configuration, see the :ref:`nrf_desktop_board_configuration_files` section.

Connection topologies
*********************

The solution supports the following ways of connecting a HID peripheral to a host:

* Directly over Bluetooth LE, where the host uses its own Bluetooth stack to attach the device as a HID over GATT peripheral.
* Directly over USB, where the device is attached as a USB HID class device.
* Over Bluetooth LE through an nRF Desktop dongle, where the dongle forwards the reports to the host over USB.

A peripheral that supports both transports can be connected in more than one way at a time.
The application selects the transport used for sending HID reports and routes the reports accordingly.
For more information, see the :ref:`nrf_desktop_usb` and :ref:`nrf_desktop_ble` sections.

Firmware architecture
*********************

The nRF Desktop application is modular and event-driven.
Functionality is split into isolated modules that communicate through application events instead of calling each other directly.

The architecture is built around the following |NCS| components:

* :ref:`Common Application Framework (CAF) <nrf:lib_caf>` provides the modules that are common to connected application designs, such as button, LED, sensor, and power management modules.
* :ref:`Application Event Manager <nrf:app_event_manager>` provides the event-based communication between modules.

A module registers as a listener of the events that it reacts to.
An event can have multiple sources and multiple listeners, which keeps modules independent of one another and makes it possible to enable only the modules that a given device role needs.

Because the code is shared across roles, the set of enabled modules is what differentiates a mouse from a keyboard or a dongle.
For the module sets used by each role, see the :ref:`nrf_desktop_architecture` section, and for the documentation of individual modules, see the :ref:`nrf_desktop_app_internal_modules` page.

Bluetooth LE connectivity
*************************

HID peripherals use the :ref:`GATT HID Service <nrf_desktop_bluetooth_guide_peripheral>` to report data to the connected host.
Depending on the SoC and the configuration, Bluetooth LE uses either Nordic Semiconductor's SoftDevice Link Layer or Zephyr's software link layer.

The SoftDevice Link Layer supports the Low Latency Packet Mode (LLPM), a proprietary extension that allows for a connection interval of 1 ms instead of the 7.5 ms minimum defined by the Bluetooth specification.
LLPM is used in configurations where the HID report rate is critical, such as gaming mice, and it requires both the peripheral and the central to support it.

The application also integrates the following host pairing experiences:

* `Fast Pair`_, for a simplified pairing flow on Android hosts.
  See the :ref:`nrf_desktop_bluetooth_guide_fast_pair` section for the supported configurations.
* `Swift Pair`_, for a simplified pairing flow on Windows hosts.

For more information about the Bluetooth configuration and the related modules, see the :ref:`nrf_desktop_bluetooth_guide` page.

Runtime configuration
*********************

The solution includes a :ref:`nrf_desktop_config_channel` that lets a host tool read and write firmware parameters while the device is running.
The channel is transported over HID feature reports, so it does not require a dedicated interface or a separate connection.

The configuration channel provides the following capabilities:

* Discovery of the device, its board name, and the modules that expose configurable options.
* Reading and writing of module options, such as motion sensor CPI or Bluetooth peer settings.
* Transfer of firmware update images.

Devices are handled in the same way whether they are connected to the host directly or through a dongle.
During discovery, the host tool queries a dongle for the peripherals connected to it and prepares them for configuration as well.

The :ref:`nrf_desktop_config_channel_script` is the reference host implementation of the channel.
The same channel is used by the Nordic HID plugin of ``fwupd``, which is described in the :ref:`nrf_desktop_fwupd` section.

Firmware updates
****************

The application supports background Device Firmware Upgrade (DFU).
The update image is transferred while the device operates normally and is stored in a dedicated update partition of the non-volatile memory.
The update is applied on the next reboot, which keeps the device usable during the transfer.

Depending on the board and the configuration, the firmware update uses one of the following bootloaders and transports:

* MCUboot in the direct-xip mode (``MCUBOOT+XIP``), which is the default for most of the currently supported boards.
* MCUboot in the RAM load mode, which executes the application from RAM to improve the HID report rate over USB.
* The immutable bootloader (B0), which is used by part of the nRF52 Series configurations.

Images can be transferred either through the :ref:`configuration channel <nrf_desktop_dfu>` or through the Simple Management Protocol, as described in the :ref:`nrf_desktop_dfu_mcumgr` section.

On boards with a hardware Key Management Unit (KMU) or Internal Trusted Storage (ITS), the configurations enable hardware cryptography for MCUboot and verify the application image with a pure ED25519 signature.

For more information, see the :ref:`nrf_desktop_bootloader` and :ref:`nrf_desktop_bootloader_background_dfu` sections.

Adapting the solution to your hardware
**************************************

The nRF Desktop application is a reference design, which means that you can use it as a starting point for your own product.

To adapt the solution, you typically perform the following tasks:

1. Add a board configuration for your hardware, as described in the :ref:`porting_guide_adding_board` section.
#. Select the device role and the transports that your product uses, and enable the corresponding modules.
#. Add support for the input hardware that your design uses, such as a different motion sensor, as described in the :ref:`porting_guide_adding_sensor` section.
#. Configure the memory layout and the bootloader for the update strategy that you want to support.
   See the :ref:`nrf_desktop_memory_layout` page for details.

|config|

Next steps
**********

After you get familiar with the solution, continue with the following pages:

* :ref:`hid_setup` for the hardware and software requirements, and for the workspace setup instructions.
* :ref:`nrf_desktop_user_interface` for a description of how a device programmed with the preconfigured firmware behaves.
* :ref:`nrf_desktop_testing_steps` for the steps that verify a device after you build and program it.
