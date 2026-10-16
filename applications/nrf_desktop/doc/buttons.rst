.. _nrf_desktop_buttons:

Buttons module
##############

.. contents::
   :local:
   :depth: 2

The buttons module is responsible for generating events related to key presses.
The source of events are changes to GPIO pins.

Module events
*************

.. include:: event_propagation.rst
    :start-after: table_buttons_start
    :end-before: table_buttons_end

.. note::
    |nrf_desktop_module_event_note|

Implementation details
**********************

nRF Desktop uses a buttons module from :ref:`nrf:lib_caf` (CAF).
Currently, all of the configurations use the standard :ref:`CAF buttons module <nrf:caf_buttons>` from the |NCS|.
To ensure lower latency, you can enable the :ref:`CAF buttons ISR module <caf_buttons_isr>` (:kconfig:option:`CONFIG_CAF_BUTTONS_ISR`).
See the linked documentation pages for implementation details.

Key ID
======

If :option:`CONFIG_DESKTOP_FN_KEYS_ENABLE` is enabled, the :c:member:`button_event.key_id` additionally encodes if a button related to :c:struct:`button_event` is a function key.
See the :ref:`nRF Desktop function key module <nrf_desktop_fn_keys>` page for implementation details.
