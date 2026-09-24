.. _setup:

Requirements and setup
######################

.. contents::
   :local:
   :depth: 2

This page outlines the requirements that you need to meet before you start working with the |addon|.

Hardware requirements
*********************

To use the |addon|, you need a development kit with an nRF54L Series SoC.
The following table lists the supported hardware platforms and their board targets:

.. table-from-rows:: /includes/sample_board_rows.txt
   :header: heading
   :rows: nrf54l15dk_nrf54l15_cpuapp, nrf54l15dk_nrf54l10_cpuapp, nrf54l15dk_nrf54l05_cpuapp, nrf54lc10dk_nrf54lc10a_cpuapp, nrf54lm20dk_nrf54lm20a_cpuapp, nrf54lm20dk_nrf54lm20b_cpuapp, nrf54ls05dk_nrf54ls05a_cpuapp, nrf54ls05dk_nrf54ls05b_cpuapp

A single board is enough to build and run a HID peripheral that connects directly to a host over Bluetooth® LE or USB (if supported by the board).
To evaluate a HID peripheral that communicates with the host through a dongle, you need the following two boards:

* With a peripheral configuration.
* With a dongle configuration.
  The board that is used as a dongle must support USB.

For details about the configuration of each supported board, see the :ref:`nrf_desktop_board_configuration_files` section.

Software requirements
*********************

To work with the |addon|, you need to install the |NCS|, including all its prerequisites and the |NCS| toolchain.
Follow the `Installing the nRF Connect SDK`_ instructions, with the following exception:

.. tabs::

   .. group-tab:: |nRFVSC|

      1. Ensure you have installed `Visual Studio Code`_ and the `nRF Connect for Visual Studio Code`_ extension.
      #. Open the nRF Connect extension in Visual Studio Code by clicking its icon in the **Activity Bar**.
      #. In the extension's **Welcome View**, click :guilabel:`Create a new application`.
      #. Select :guilabel:`Browse nRF Connect SDK Add-on Index`, then choose :guilabel:`HID`.
      #. Select v1.0.0 of the |addon|.

   .. group-tab:: Command line

      .. tabs::

         .. group-tab:: Initialize a new workspace

            1. Run the following command to initialize west with the |addon| v1.0.0, which also initializes the |NCS| v3.4.1:

               .. code-block:: console

                  west init -m https://github.com/nrfconnect/sdk-hid --mr v1.0.0

            #. Update the |NCS| modules:

               .. code-block:: console

                  west update

         .. group-tab:: Include the add-on in an existing |NCS| workspace

            1. Assuming you have an existing |NCS| workspace in the :file:`ncs` folder, run the following commands:

               a. Navigate to the workspace folder:

                  .. code-block:: console

                     cd ncs

               #. Clone the add-on repository into the :file:`hid` folder, which is the path expected by the add-on manifest:

                  .. code-block:: console

                     git clone --branch v1.0.0 https://github.com/nrfconnect/sdk-hid hid

               #. Set the manifest path to the add-on directory:

                  .. code-block:: console

                     west config manifest.path hid

               #. Update the |NCS| modules:

                  .. code-block:: console

                     west update

            2. Optionally, run these commands in case you need to work on the |NCS| without the add-on:

               a. Configure the manifest path back to the |NCS| directory:

                  .. code-block:: console

                     west config manifest.path nrf

               #. Update |NCS| modules:

                  .. code-block:: console

                     west update

               #. Check the current manifest path with the following command:

                  .. code-block:: console

                     west config manifest.path

                  The output should be:

                  .. code-block:: console

                     nrf

                  This means that the current workspace is using the |NCS|.
