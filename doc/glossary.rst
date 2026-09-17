.. _hid_glossary:

Glossary
*********

The glossary defines the key terms used throughout the documentation.

.. glossary::
   :sorted:

   Human Interface Device (HID)
      A Human Interface Device (HID) is a type of computer device that takes input from or provides output to a computer user.
      Examples of such devices are keyboards, mice, game controllers, and touchscreens.

   HID SCI (HID Shorter Connection Intervals)
      A standardized mechanism defined in the `HID Over GATT Profile Specification`_ that enables shorter Bluetooth LE connection intervals than the standard 7.5 ms minimum.
      In the nRF Desktop application, HID SCI is used to achieve higher HID report rates while remaining compliant with the Bluetooth specification.

   LLPM (Low Latency Packet Mode)
      A proprietary Bluetooth extension from Nordic Semiconductor that enables 1 ms connection intervals.
      LLPM can be used only when it is supported by both connected devices.
      In the nRF Desktop application, LLPM is used to achieve high HID report rates (up to 1000 reports in second), which is not supported by standard Bluetooth LE connection parameters.
