.. _setup:

Requirements and setup
######################

.. contents::
   :local:
   :depth: 2

This page outlines the requirements that you need to meet before you start working with the |hid_addon|.

Hardware requirements
*********************

To use the |hid_addon|, you need a development kit with an nRF54L Series SoC.
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


Get the |hid_addon| code
************************

The |hid_addon| is distributed as a Git repository and is managed through its own west manifest.
The compatible |NCS| version is specified in the :file:`west.yml` file.
Follow the `HID Add-on GitHub repository`_ link to browse the codebase.
To get the |hid_addon| code, pick your method from the following:

* Use the `nRF Connect for Visual Studio Code`_ extension, which provides a convenient way to clone the add-on and compatible |NCS| version.
* Clone the add-on repository and initialize west with the add-on manifest.
* Extend your west manifest with Add-on as a project.
* Clone the Add-on repository and add the Add-on as a Zephyr module through CMake or environment variable.
* Switch the west manifest to the Add-on in an existing |NCS| workspace.

.. tabs::

   .. group-tab:: |nRFVSC|

      .. note::
         Use this method when you wish to specifically evaluate nRF Desktop or other tools provided by the Add-on, but do not have the |NCS| setup yet.

      Clone the |hid_addon| code, together with the compatible |NCS|:

      1. Ensure you have installed `Visual Studio Code`_ and the `nRF Connect for Visual Studio Code`_ extension.
      #. Follow the :ref:`nRF Connect SDK installation guide <nrf:install_ncs>` to install |NCS| prerequisites and toolchain |toolchain_ncs_id|.

         .. note::
            The compatible version of the |NCS| will be cloned with the |hid_addon| repository in the following steps.
            The version of |NCS| is fixed to the version of the |hid_addon| and is hard-coded in the :file:`west.yml` file of the |hid_addon|.

      #. Open the nRF Connect extension in Visual Studio Code by clicking its icon in the **Activity Bar**.
      #. In the extension's **Welcome View**, click :guilabel:`Create a new application`.
         The list of actions appears in Visual Studio Code's quick pick.
      #. Click :guilabel:`Browse nRF Connect SDK Add-on Index`.
         The list of available |NCS| add-ons appears in Visual Studio Code's quick pick.
      #. Select **HID**.
      #. Select the Add-on version to install.
         Depending on the speed of your internet connection, the update might take some time.

   .. group-tab:: Command line for local manifest

      .. note::
         Use this method when you wish to specifically evaluate nRF Desktop or other tools provided by the Add-on, but do not have the |NCS| setup yet.

      1. Follow the :ref:`nRF Connect SDK installation guide <nrf:install_ncs>` to install |NCS| prerequisites and toolchain |toolchain_ncs_id|.

         .. note::
            The compatible version of the |NCS| will be cloned with the |hid_addon| repository in the following steps.
            The version of |NCS| is fixed to the version of the |hid_addon| and is hard-coded in the :file:`west.yml` file of the |hid_addon|.

      #. Launch the installed toolchain:

         .. tabs::

            .. group-tab:: Windows

               .. code-block:: console

                  nrfutil sdk-manager toolchain launch --ncs-version |toolchain_ncs_id| --terminal

            .. group-tab:: Linux

               .. code-block:: console

                  nrfutil sdk-manager toolchain launch --ncs-version |toolchain_ncs_id| --shell

            .. group-tab:: macOS

               .. code-block:: console

                  nrfutil sdk-manager toolchain launch --ncs-version |toolchain_ncs_id| --shell

      #. Initialize the |hid_addon| repository using one of the following methods:

         .. tabs::

            .. tab:: Direct initialization (Recommended)

               a. Initialize west with the remote manifest:

                  .. code-block:: console

                     west init -m https://github.com/nrfconnect/ncs-hid --mr v1.0.0

            .. tab:: Manual cloning and initialization

               a. Clone the |hid_addon| repository into the :file:`hid` folder, which is the path expected by the add-on manifest:

                  .. code-block:: console

                     git clone --branch v1.0.0 https://github.com/nrfconnect/ncs-hid hid

               #. Initialize west with the local manifest:

                  .. code-block:: console

                     west init -l hid

      #. Update all repositories by running the following command:

         .. code-block:: console

            west update

         Depending on the speed of your internet connection, the update might take some time.

   .. group-tab:: Add-on as a manifest project

      .. note::
         Use this method when running a :ref:`zephyr:zephyr-workspace-app` or if you prefer to use a modification of the |NCS| manifest.

      1. Add the |hid_addon| repository as a project in the west manifest by including the following lines in your :file:`west.yml` under the ``projects`` key:

         .. code-block:: yaml

            - name: hid
              url: https://github.com/nrfconnect/ncs-hid
              revision: v1.0.0
              import: true

         If you have already included the |NCS| in your west manifest, remove it or replace the above ``import`` key with the mapping:

         .. code-block:: yaml

            import:
               name-blocklist:
               - nrf

         Your west manifest must specify compatible versions of the |NCS| and Add-on.

      #. Run ``west update`` to pull the |hid_addon| repository.

   .. group-tab:: Add-on as an extra Zephyr module

      .. note::
         Use this method if you have a former installation of the |NCS| and would like to evaluate or use the |hid_addon| with that installation.
         This method allows you to keep the |NCS| manifest unmodified.

      Before using this approach, ensure that a compatible version of the |NCS| is installed.
      To identify the compatible |NCS| version, check the :file:`west.yml` file of the |hid_addon|.
      Since west does not manage the Add-on in this setup, you are responsible for keeping the versions synchronized.

      1. Clone the Add-on repository:

         .. code-block:: console

            git clone --branch v1.0.0 https://github.com/nrfconnect/ncs-hid

      #. Set the CMake or environment variable ``EXTRA_ZEPHYR_MODULES`` to the Add-on code path.
         Use absolute path to ensure proper path resolution.
         Check the :ref:`zephyr:env_vars` documentation for different ways of setting environment variables in Zephyr.

   .. group-tab:: Switch west manifest to the Add-on

      .. note::
         Use this method if you already have an |NCS| workspace and want west to use the |hid_addon| :file:`west.yml` as the workspace manifest instead of the |NCS| one.
         You can switch back to the |NCS| manifest at any time.

      1. Assuming you have an existing |NCS| workspace in the :file:`ncs` folder, run the following commands:

         a. Navigate to the workspace folder:

            .. code-block:: console

               cd ncs

         #. Clone the add-on repository into the :file:`hid` folder, which is the path expected by the add-on manifest:

            .. code-block:: console

               git clone --branch v1.0.0 https://github.com/nrfconnect/ncs-hid hid

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
