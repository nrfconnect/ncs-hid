.. _index:

|addon| for |NCS|
#################

.. contents::
   :local:
   :depth: 2

.. note::
   This is an add-on for :ref:`nRF Connect SDK <nrf:index>`.

The |addon| provides a complete solution for developing a :term:`Human Interface Device (HID)` on Nordic Semiconductor SoCs.
A Human Interface Device (HID) is a type of computer device that takes input from or provides output to a computer user.
Examples of such devices are keyboards, mice, game controllers, and touchscreens.
The add-on combines device-side firmware, based on the nRF Desktop reference design, with host-side tools that configure and update the device at runtime.

This page describes the parts of the solution and how they work together.
For the requirements that you need to meet before you start, see the :ref:`setup` page.

The software in the |addon| has Supported maturity level unless explicitly stated otherwise for a given software component.

Solution components
*******************

The |addon| consists of the following components:

nRF Desktop application
   The :ref:`nrf_desktop` HID reference design application, which is the firmware that runs on the device.
   A single code base covers all supported device roles, and the role is selected through the application configuration.
   Application firmware is based on the Zephyr RTOS and the |NCS|.

HID configurator scripts
   The :ref:`nrf_desktop_config_channel_script` host tool, which is a set of Python scripts that run on a PC.
   The scripts discover connected nRF Desktop devices, read and write their runtime options, and perform firmware updates.

   You can also update device firmware through `Linux Vendor Firmware Service (LVFS) <LVFS_>`_ and `fwupd`_.
   See the :ref:`nrf_desktop_fwupd` documentation page for details.

See the subpages for detailed documentation.

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: Subpages:

   setup
   ../applications/nrf_desktop/README
   ../scripts/hid_configurator/README
   libraries/caf/index
   glossary
   release_notes
