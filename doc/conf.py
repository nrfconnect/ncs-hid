# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

import os
import sys
from pathlib import Path

import west.manifest

manifest = west.manifest.Manifest.from_topdir()

HID_BASE = Path(manifest.repo_abspath)
NRF_BASE = Path(manifest.get_projects(['nrf'])[0].abspath)
ZEPHYR_BASE = Path(manifest.get_projects(['zephyr'])[0].abspath)

# The add-on's own extensions must take precedence: page_filter is vendored here
# so that it reads hid/doc/versions.json instead of the nRF Connect SDK copy,
# which would populate the version filter with |NCS| releases.
sys.path.insert(0, str(NRF_BASE / 'doc' / '_extensions'))
sys.path.insert(0, str(ZEPHYR_BASE / 'doc' / '_extensions'))
sys.path.insert(0, str(HID_BASE / 'doc' / '_extensions'))

# Needed by options_from_kconfig extension which is not self contained
sys.path.insert(0, str(ZEPHYR_BASE / 'scripts'))

from hid_project_info import (
    get_manifest_revision,
    get_version_string,
    strip_v,
    build_rst_epilog,
)

# -- Project information -----------------------------------------------------

project = 'nRF Connect SDK - HID Add-on'
copyright = '2026, Nordic Semiconductor'
author = 'Nordic Semiconductor'

# The full version, including alpha/beta/rc tags.
# Read from hid/VERSION via the canonical Zephyr-format parser.
# 'release' is a special symbol used by Sphinx for the version string in docs.
release = get_version_string(HID_BASE / 'VERSION')


# -- Version information from west manifest ---------------------------------

NCS_VERSION              = get_manifest_revision(manifest, "nrf")
NCS_VERSION_NUMBER       = strip_v(NCS_VERSION)
TOOLCHAIN_NCS_ID         = NCS_VERSION
ADDON_RELEASE            = release

# Zephyr version — read from zephyr/VERSION (same format as hid/VERSION).
ZEPHYR_VERSION_NUMBER    = get_version_string(ZEPHYR_BASE / "VERSION")
ZEPHYR_VERSION           = f"v{ZEPHYR_VERSION_NUMBER}"


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.intersphinx',
    'sphinx_tabs.tabs',
    'sphinx_copybutton',
    'sphinxcontrib.mscgen',
    'options_from_kconfig',
    'table_from_rows',
    'page_filter',
    'zephyr.doxyrunner',
    'zephyr.doxybridge',
    'zephyr.external_content',
    'zephyr.kconfig',
    'sphinxcontrib.plantuml',
    'hid_project_info',
]

plantuml = 'plantuml'
plantuml_output_format = 'svg_img'

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store', 'index_*.rst']

# Options for external_content -------------------------------------------------

external_content_contents = [
    (HID_BASE / "doc", "[!_]*"),
    (HID_BASE, "applications/**/*.rst"),
    (HID_BASE, "scripts/**/README.rst"),
    (HID_BASE, "samples/**/*.rst"),
    (HID_BASE, "tests/**/*.rst"),
    (HID_BASE, "lib/**/Kconfig"),
    (HID_BASE, "subsys/**/Kconfig"),
    (HID_BASE, "subsys/**/Kconfig.*"),
]

# -- Options for doxyrunner plugin ---------------------------------------------

build_dir = sys.argv[4]
_doxyrunner_outdir = Path(build_dir) / "html" / "doxygen"
doxyrunner_doxygen = os.environ.get("DOXYGEN_EXECUTABLE", "doxygen")
doxyrunner_projects = {
    "hid": {
        "doxyfile": HID_BASE / "doc" / "doxyfile.in",
        "outdir": _doxyrunner_outdir,
        "fmt": True,
        "fmt_vars": {
            "NRF_BASE": str(NRF_BASE),
            "HID_BASE": str(HID_BASE),
            "DOCSET_SOURCE_BASE": str(HID_BASE),
            "DOCSET_BUILD_DIR": str(_doxyrunner_outdir),
            "DOCSET_VERSION": release,
        },
    }
}

# -- Options for zephyr.doxybridge plugin ---------------------------------

doxybridge_projects = {"hid": doxyrunner_projects["hid"]["outdir"]}

# Options for table_from_rows --------------------------------------------------

table_from_rows_base_dir = HID_BASE
table_from_sample_yaml_board_reference = "/includes/sample_board_rows.txt"

# Options for options_from_kconfig ---------------------------------------------

options_from_kconfig_base_dir = HID_BASE
options_from_kconfig_zephyr_dir = ZEPHYR_BASE

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_ncs_theme'

html_theme_options = {
    'docsets': {},
    "ncs_url": "https://nrfconnectdocs.nordicsemi.com/ncs/latest/nrf/",
    "ncs_label": "nRF Connect SDK Docs",
    "addons_url": "https://nrfconnect.github.io/ncs-app-index/",
    "bare_metal_url": "",
    "logo_url": "https://docs.nordicsemi.com",
}

html_show_sphinx = False
html_extra_path = ['versions.json']

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = [str(HID_BASE / "doc" / "_static")]
html_css_files = ['custom.css']

# -- Project-wide string substitutions ---------------------------------------
#
# Single source of truth for project-wide string substitutions (the tokens
# referenced in RST source as ``|name|``).
#
# Each entry is a ``(name, value)`` pair consumed by the
# ``hid_project_info`` extension, which:
#
#   1. Validates uniqueness of names at builder-init time.
#
#   2. Pre-expands ``|name|`` markers in raw RST source *before* docutils
#      parses it (via a ``source-read`` hook).  This makes substitutions
#      work uniformly everywhere, including inside ``code-block`` and
#      ``literalinclude`` bodies.
#
#   3. Is used here via ``build_rst_epilog`` to pre-expand the same
#      markers inside ``links.txt`` / ``shortcuts.txt`` URI targets before
#      the text is handed to Sphinx as ``rst_epilog``.  (Docutils does not
#      expand substitution references inside hyperlink target URIs.)
#
# List longer names before any name that is a substring of another to
# ensure the longer match is applied first (the surrounding ``|``
# delimiters already prevent overlap today, but the convention keeps the
# file robust against future marker-syntax changes).
#
# Add new substitutions here only — do not redefine them elsewhere.

SUBSTITUTIONS = [
    ("release_version",             ADDON_RELEASE),
    ("ncs_version_number",          NCS_VERSION_NUMBER),
    ("ncs_version",                 NCS_VERSION),
    ("toolchain_ncs_id",            TOOLCHAIN_NCS_ID),
    ("zephyr_version_number",       ZEPHYR_VERSION_NUMBER),
    ("zephyr_version",              ZEPHYR_VERSION),
]

# Consumed by the hid_project_info extension (source-read handler).
hid_substitutions = SUBSTITUTIONS

# -- Options for intersphinx --------------------------------------------------
#
# Hosted NCS inventories are published at nrfconnectdocs.nordicsemi.com.
# See https://www.sphinx-doc.org/en/master/usage/extensions/intersphinx.html

_NCS_DOCS_BASE = f"https://nrfconnectdocs.nordicsemi.com/ncs/{NCS_VERSION_NUMBER}"
_NCS_DOCS_STORAGE = f"https://ncsdoc.z6.web.core.windows.net/ncs/{NCS_VERSION_NUMBER}"

intersphinx_mapping = {
    "nrf": (f"{_NCS_DOCS_BASE}/nrf/", f"{_NCS_DOCS_STORAGE}/nrf/objects.inv"),
    "kconfig": (f"{_NCS_DOCS_BASE}/kconfig/", f"{_NCS_DOCS_STORAGE}/kconfig/objects.inv"),
    "mcuboot": (f"{_NCS_DOCS_BASE}/mcuboot/", f"{_NCS_DOCS_STORAGE}/mcuboot/objects.inv"),
    "zephyr": (f"{_NCS_DOCS_BASE}/zephyr/", f"{_NCS_DOCS_STORAGE}/zephyr/objects.inv"),
}

# -- rst_epilog: pre-expanded links.txt + shortcuts.txt ----------------------

rst_epilog = build_rst_epilog(HID_BASE / "doc", SUBSTITUTIONS)
