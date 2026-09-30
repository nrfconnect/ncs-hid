:orphan:

.. _known_issues:

Known issues
############

.. contents::
   :local:
   :depth: 2

Known issues listed on this page *and* tagged with the :ref:`latest release version <release_notes>` are valid for the current state of development.
Use the drop-down filter to see known issues for previous releases and check if they are still valid.

Items can have one or both of the following entries:

* **Affected platforms:**

  If a known issue does not have any specific platforms listed, it is valid for all hardware platforms.

* **Workaround:**

  Some known issues have a workaround.
  Sometimes, they are discovered later and added over time.

The add-on inherits the known issues that are related to its dependencies like HIDS (HID GATT Service), Fast Pair, and so on from the `nRF Connect SDK known issues`_.

List of known issues
********************

.. version-filter::
  :default: v1-0-0
  :container: dl/dt
  :tags: [("wontfix", "Won't fix")]

.. page-filter::
  :name: issues

  wontfix    Won't fix

.. rst-class:: wontfix v1-0-0

NCSDK-35817: The HID configurator returns an Input/Output error (EIO) in the Linux environment during a long exchange of HID feature reports
  The BlueZ stack passes an incorrect report exchange identifier to the Linux Userspace HID driver (UHID), which causes an EIO.
  The issue is caused by the 16-bit unsigned integer overflow and appears after devices exchange more than 65535 HID feature reports.
  The 16-bit variable overflow can be observed in BlueZ versions 5.73 or lower.

  The overflow for BlueZ versions from 5.74 to 5.84 happens even earlier.
  It appears after devices exchange more than 255 HID feature reports (8-bit unsigned integer overflow).

  For more details, see the `Ubuntu bug report`_.

  **Workaround:** Build the BlueZ stack from sources using the ``master`` branch to replace your default BlueZ package.
  Alternatively, use the ``sudo systemctl restart bluetooth`` command to restart the counter used to identify HID feature report exchanges.
  After using the command, an interrupted configuration channel DFU operation can be resumed.

.. rst-class:: wontfix v1-0-0

NCSDK-8304: HID configurator issues for peripherals connected over Bluetooth LE to Linux host
  Using :ref:`nrf_desktop_config_channel_script` for peripherals connected to host directly over Bluetooth LE might result in receiving improper HID feature report ID.
  In such case, the device will provide HID input reports, but it cannot be configured with the HID configurator.

  **Workaround:** Use BlueZ in version 5.56 or higher.

.. note::
   nRF Desktop application is also affected by the nRF Connect SDK's Fast Pair and Bluetooth HID sample issues ``NCSDK-19942``, ``NCSDK-26669``, ``NCSDK-34682``, and ``NCSDK-38735``.
   See the :ref:`nrf:known_issues` for details.
