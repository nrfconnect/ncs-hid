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
Most configurations use the standard :ref:`CAF buttons module <nrf:caf_buttons>` from the |NCS|.
The HID SCI peripheral build types (``hid_sci``, ``release_hid_sci``, ``hid_sci_keyboard``, and ``release_hid_sci_keyboard``) use the :ref:`CAF buttons ISR module <caf_buttons_isr>` (:option:`CONFIG_CAF_BUTTONS_ISR`).
For other configurations, you can enable the CAF buttons ISR module to ensure lower overall click latency.
See the linked documentation pages for implementation details.

Key ID
======

If :option:`CONFIG_DESKTOP_FN_KEYS_ENABLE` is enabled, the :c:member:`button_event.key_id` additionally encodes if a button related to :c:struct:`button_event` is a function key.
See the :ref:`nRF Desktop function key module <nrf_desktop_fn_keys>` page for implementation details.
