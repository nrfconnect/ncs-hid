/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>

#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <ram_pwrdn.h>

#define MODULE ram_pwrdn
#include <caf/events/module_state_event.h>

LOG_MODULE_REGISTER(desktop_ram_pwrdn, CONFIG_DESKTOP_RAM_PWRDN_LOG_LEVEL);

extern char _image_ram_end[];

static void desktop_ram_power_down_mcuboot_region(void)
{
#if defined(CONFIG_MCUBOOT_BOOTLOADER_MODE_RAM_LOAD) && \
	defined(CONFIG_DESKTOP_MCUBOOT_RAM_LOAD_MCUBOOT_RAM_SIZE)
	uintptr_t mcuboot_start = CONFIG_DESKTOP_MCUBOOT_RAM_LOAD_MCUBOOT_RAM_START_ADDR;
	uintptr_t mcuboot_end = mcuboot_start + CONFIG_DESKTOP_MCUBOOT_RAM_LOAD_MCUBOOT_RAM_SIZE;
	uintptr_t image_ram_end = (uintptr_t)_image_ram_end;
	uintptr_t start = MAX(mcuboot_start, image_ram_end);

	if (start >= mcuboot_end) {
		return;
	}

	LOG_INF("Power down MCUboot RAM 0x%08x-0x%08x", (uint32_t)start, (uint32_t)mcuboot_end);
	power_down_ram(start, mcuboot_end);
#endif
}

static void desktop_ram_power_down_explicit_unused_pool(void)
{
#if defined(CONFIG_DESKTOP_RAM_LOAD_UNUSED_RAM_SIZE)
	uintptr_t image_ram_end = (uintptr_t)_image_ram_end;
	uintptr_t unused_start = CONFIG_DESKTOP_RAM_LOAD_UNUSED_RAM_START_ADDR;
	uintptr_t unused_end = unused_start + CONFIG_DESKTOP_RAM_LOAD_UNUSED_RAM_SIZE;

	if (image_ram_end < unused_start) {
		LOG_INF("Power down app executable region tail 0x%08x-0x%08x",
			(uint32_t)image_ram_end, (uint32_t)unused_start);
		power_down_ram(image_ram_end, unused_start);
	}

	LOG_INF("Power down unused pool 0x%08x-0x%08x", (uint32_t)unused_start,
		(uint32_t)unused_end);
	power_down_ram(unused_start, unused_end);
#endif
}

static void desktop_ram_pwrdn_apply(void)
{
	desktop_ram_power_down_explicit_unused_pool();
	desktop_ram_power_down_mcuboot_region();
	power_down_unused_ram();
}

static bool app_event_handler(const struct app_event_header *aeh)
{
	if (is_module_state_event(aeh)) {
		const struct module_state_event *event =
				cast_module_state_event(aeh);

		if (check_state(event, MODULE_ID(main), MODULE_STATE_READY)) {
			static bool initialized;

			__ASSERT_NO_MSG(!initialized);
			initialized = true;

			desktop_ram_pwrdn_apply();
			module_set_state(MODULE_STATE_READY);
		}

		return false;
	}

	__ASSERT_NO_MSG(false);

	return false;
}

APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE_EARLY(MODULE, module_state_event);
