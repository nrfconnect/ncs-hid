.. _nrf_desktop_api:

nRF Desktop: API documentation
##############################

.. contents::
   :local:
   :depth: 2

Following are the API elements used by the application.

HID reports
***********

| Header file: :file:`applications/nrf_desktop/configuration/common/hid_report_desc.h`
| Source file: :file:`applications/nrf_desktop/configuration/common/hid_report_desc.c`

.. doxygengroup:: nrf_desktop_hid_reports

HID report provider events
**************************

| Header file: :file:`applications/nrf_desktop/src/events/hid_report_provider_event.h`
| Source file: :file:`applications/nrf_desktop/src/events/hid_report_provider_event.c`

.. doxygengroup:: nrf_desktop_hid_report_provider_event

LED states
**********

| Header file: :file:`applications/nrf_desktop/configuration/common/led_state.h`
| Source file: :file:`applications/nrf_desktop/src/modules/led_state.c`

.. doxygengroup:: nrf_desktop_led_state

Motion events
*************

| Header file: :file:`applications/nrf_desktop/src/events/motion_event.h`
| Source file: :file:`applications/nrf_desktop/src/events/motion_event.c`

.. doxygengroup:: nrf_desktop_motion_event

USB events
**********

| Header file: :file:`applications/nrf_desktop/src/events/usb_event.h`
| Source file: :file:`applications/nrf_desktop/src/events/usb_event.c`

.. doxygengroup:: nrf_desktop_usb_event

CAF Bluetooth LE common event extension
***************************************

| Header file: :file:`include/caf/events/ble_common_event_extension.h`
| Source file: :file:`subsys/caf/events/ble_common_event_extension.c`

.. doxygengroup:: caf_ble_common_event_extension

.. note::
   The |addon| locally extends CAF Bluetooth LE common events to provide events that are not yet available in the used nRF Connect SDK release.
