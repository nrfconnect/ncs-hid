.. _nrf_desktop_porting_guide:

nRF Desktop: Integrating your own hardware
##########################################

.. contents::
   :local:
   :depth: 2

This page describes how to adapt the nRF Desktop application to different hardware and lists the required steps.

.. tip::
   Make sure to get familiar with the :ref:`nrf_desktop_configuration` section before porting the application to a new board.

.. _porting_guide_adding_board:

Adding a new board
******************

When adding a new board for the first time, focus on a single configuration.
Moreover, keep the default ``debug`` build type that the application is built with, and do not add any additional build type parameters.
The following procedure uses the gaming mouse configuration as an example.

Zephyr support for a board
==========================

Before introducing nRF Desktop application configuration for a given board, you need to ensure that the board is supported in Zephyr.

.. note::
   You can skip this step if your selected board is already supported in Zephyr.

Follow the Zephyr's :ref:`zephyr:board_porting_guide` for detailed instructions related to introducing Zephyr support for a new board.
Make sure that the following conditions are met:

* Edit the DTS files to make sure they match the hardware configuration.
   Pay attention to the following elements:

   * Pins that are used.
   * Bus configuration for optical sensor.
   * `Changing interrupt priority`_.

* Edit the board's Kconfig files to make sure they match the required system configuration.
   For example, disable the drivers that will not be used by your device.

.. tip::
   You can define the new board by copying the nRF Desktop reference design files that are the closest match for your hardware and then aligning the configuration to your hardware.
   For example, for gaming mouse use :file:`nrf/boards/nordic/nrf54lm20dk/nrf54lm20a/cpuapp`.

nRF Desktop support for a board
===============================

Perform the following steps to add nRF Desktop application configuration for a board that is already supported in Zephyr.

1. Copy the project files for the device that is the closest match for your hardware.
   For example, for a mouse these are located at :file:`applications/nrf_desktop/configuration/nrf54lm20dk/nrf54lm20a/cpuapp`.
#. Optionally, depending on the reference design, edit the DTS overlay file.
   This step is not required if you have created a new reference design and its DTS files fully describe your hardware.
   In such case, the overlay file can be left empty.
#. In Kconfig, ensure that the hardware interface modules required by your device are enabled.
   For a mouse, this typically includes :ref:`caf_buttons`, :ref:`caf_leds`, and :ref:`nrf_desktop_motion`.
#. For each module enabled, change its configuration to match your hardware.
   Apply the following changes, depending on the module:

   Motion module
     * The nRF54 Series DK mouse configurations use motion buttons.
       Configure the button key IDs in the application configuration.
   Buttons module
     * To simplify the configuration of arrays, the nRF Desktop application uses :file:`_def` files.
     * The :file:`_def` file of the buttons module contains pins assigned to rows and columns.
   LEDs module
     * The application uses two logical LEDs - one for the peers state, and one for the system state indication.
     * Each of the logical LEDs can have either one (monochromatic) or three color channels (RGB).
       Such color channel is a physical LED.
     * The module uses Zephyr's :ref:`zephyr:led_api` driver for setting the LED color.
       Zephyr's LED driver can use the implementation based on either GPIO or PWM (Pulse-Width Modulation).
       The hardware configuration is described through DTS.
       See the :ref:`caf_leds` configuration section for details.

#. Review the :ref:`nrf_desktop_hid_configuration`.
#. By default, the nRF Desktop device enables Bluetooth connectivity support.
   Review the :ref:`nrf_desktop_bluetooth_configuration`.

   a. Ensure that the Bluetooth role is properly configured.
      For mouse, it should be configured as peripheral.
   #. Update the configuration related to peer control.
      You can also disable the peer control using the :option:`CONFIG_DESKTOP_BLE_PEER_CONTROL` option.
      Peer control details are described in the :ref:`nrf_desktop_ble_bond` documentation.

#. Edit Kconfig to disable options that you do not use.
   Some options have dependencies that might not be needed when these options are disabled.
   For example, when the LEDs module is disabled, the PWM driver is not needed.

.. _porting_guide_adding_sensor:

Adding motion input
*******************

The preconfigured nRF Desktop DK uses :ref:`nrf_desktop_motion` with motion buttons.
When porting to custom hardware with an optical motion sensor, add a dedicated hardware interface module and select it through the :file:`src/hw_interface/Kconfig.motion` file.

Changing interrupt priority
===========================

You can edit the DTS files to change the priority of the peripheral's interrupt.
This can be useful when :ref:`adding a new custom board <porting_guide_adding_board>` or whenever you need to change the interrupt priority.

The ``interrupts`` property is an array, where the meaning of each element is defined by the specification of the interrupt controller.
These specification files are located at :file:`zephyr/dts/bindings/interrupt-controller/` DTS binding file directory.

For example, for an Arm Cortex-M device, see the corresponding devicetree binding files in :file:`zephyr/dts/bindings/interrupt-controller/`.
The ``interrupts`` property is an array, where the meaning of each element is defined by the specification of the interrupt controller.
The default values for these elements for the given peripheral are in the :file:`dtsi` file specific for the device.

.. code-block::

   spi1: spi@40004000 {
           /*
            * This spi node can be SPI, SPIM, or SPIS,
            * for the user to pick:
            * compatible = "nordic,nrf-spi" or
            *              "nordic,nrf-spim" or
            *              "nordic,nrf-spis".
            */
           #address-cells = <1>;
           #size-cells = <0>;
           reg = <0x40004000 0x1000>;
           interrupts = <4 1>;
           status = "disabled";
   };

To change the priority of the peripheral's interrupt, override the ``interrupts`` property of the peripheral node by including the following code snippet in the :file:`dts.overlay` file or directly in the board DTS:

.. code-block:: none

   &spi1 {
       interrupts = <4 2>;
   };

This code snippet changes the **SPI1** interrupt priority from default ``1`` to ``2``.
