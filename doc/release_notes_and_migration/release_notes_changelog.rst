.. _release_notes_changelog:

Changelog for |hid_addon| v1.0.99
#################################

.. contents::
   :local:
   :depth: 2

The most relevant changes that are present on the main branch of the |hid_addon|, as compared to the latest official release, are tracked in this file.

.. note::
   This file is a work in progress and might not cover all relevant changes.

.. HOWTO

   When adding a new change, decide whether it needs an entry in the changelog.
   If it does, update this page.
   Add the sections you need, as only a handful of sections are kept when the changelog is cleaned for the next release.

Known issues
************

For the list of issues valid for this release, navigate to `known issues page on the main branch`_ and select the latest official tag from the dropdown list.

Changelog
*********

The following sections provide detailed lists of changes by component.

Applications
============

nRF Desktop application
=======================

* Set the CPU clock frequency to 64 MHz on nRF54L Series nRF Desktop board configurations.
  The setting is in each board's ``memory_map*.dtsi`` files, included from application and MCUboot overlays for a given build type; both images must use the same CPU clock.
  Both images run the CPU HFPLL at 64 MHz (instead of the Zephyr default of 128 MHz for nRF54L family SoCs).

Host tools
==========

|no_changes_yet_note|

Libraries and subsystems
========================

|no_changes_yet_note|
