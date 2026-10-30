/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "hid_keymap.h"
#include <caf/key_id.h>

/* This configuration file is included only once from hid_keymap module and holds
 * information about mapping between buttons and generated reports.
 */

/* This structure enforces the header file is included only once in the build.
 * Violating this requirement triggers a multiple definition error at link time.
 */
const struct {} hid_keymap_def_include_once;

static const struct hid_keymap hid_keymap[] = {
	{ KEY_ID(0, 0), 0x01, REPORT_ID_MOUSE }, /* button0 - Left Mouse Button */
	{ KEY_ID(0, 1), 0x02, REPORT_ID_MOUSE }, /* button1 - Right Mouse Button */
	{ KEY_ID(0, 2), 0x03, REPORT_ID_MOUSE }, /* button2 - Middle Mouse Button */
	{ KEY_ID(0, 3), 0x04, REPORT_ID_MOUSE }, /* button3 - Additional Mouse Button */
};
