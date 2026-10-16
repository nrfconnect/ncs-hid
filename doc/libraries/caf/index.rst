.. _caf_extensions:

CAF extension modules
#####################

The |hid_addon| provides :ref:`nrf:lib_caf` (CAF) extensions that are not yet available in the used |NCS| release.
These extensions are implemented under :file:`subsys/caf/` and will be removed once the features are available in the used |NCS| version.

The |hid_addon| also introduces the :ref:`caf_buttons_isr`.
The module is optimized for the gaming-grade mouse use case and provides lower click latency than :ref:`nrf:caf_buttons`.

For the core CAF modules, see :ref:`nrf:lib_caf` in the |NCS| documentation.

.. toctree::
   :maxdepth: 1
   :caption: Subpages:

   config
   events
   ble_state_extension
   buttons_isr
