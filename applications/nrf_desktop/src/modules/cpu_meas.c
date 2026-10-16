/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <debug/cpu_load.h>
#include <nrf_profiler.h>

#define MODULE cpu_meas
#include <caf/events/module_state_event.h>

#include "cpu_load_event.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_DESKTOP_CPU_MEAS_LOG_LEVEL);

static struct k_work_delayable cpu_load_read;
static uint16_t nrf_profiler_event_id;


static void log_cpu_load_nrf_profiler(uint32_t load)
{
	if (is_profiling_enabled(nrf_profiler_event_id)) {
		struct log_event_buf buf;

		nrf_profiler_log_start(&buf);
		nrf_profiler_log_encode_uint32(&buf, load);
		nrf_profiler_log_send(&buf, nrf_profiler_event_id);
	}
}

static void send_cpu_load_event(uint32_t load)
{
	struct cpu_load_event *event = new_cpu_load_event();

	event->load = load;
	APP_EVENT_SUBMIT(event);
}

static void cpu_load_read_fn(struct k_work *work)
{
	ARG_UNUSED(work);

	int cpu_load = cpu_load_get();

	if (cpu_load < 0) {
		/* Stop measurements after a fatal error. */
		LOG_ERR("cpu_load_get failed, err: %d", cpu_load);
		module_set_state(MODULE_STATE_ERROR);
		return;
	}

	/* Ensure result will be provided. */
	BUILD_ASSERT(IS_ENABLED(CONFIG_DESKTOP_CPU_MEAS_RESULT_NRF_PROFILER) ||
		     IS_ENABLED(CONFIG_DESKTOP_CPU_MEAS_RESULT_EVENT));

	if (IS_ENABLED(CONFIG_DESKTOP_CPU_MEAS_RESULT_EVENT)) {
		send_cpu_load_event(cpu_load);
	}

	if (IS_ENABLED(CONFIG_DESKTOP_CPU_MEAS_RESULT_NRF_PROFILER)) {
		log_cpu_load_nrf_profiler(cpu_load);
	}

	cpu_load_reset();
	(void)k_work_reschedule(&cpu_load_read, K_MSEC(CONFIG_DESKTOP_CPU_MEAS_PERIOD));
}

static void register_nrf_profiler_event(void)
{
	static const char * const data_names[] = {"load"};
	static const enum nrf_profiler_arg data_types[] = {NRF_PROFILER_ARG_U32};

	nrf_profiler_event_id = nrf_profiler_register_event_type("cpu_load",
								 data_names,
								 data_types,
								 ARRAY_SIZE(data_types));
}

static void init(void)
{
	/* When this option is enabled, CPU load measurement is periodically
	 * resetted. Only cpu_meas module should reset the measurement.
	 */
	BUILD_ASSERT(!IS_ENABLED(CONFIG_NRF_CPU_LOAD_LOG_PERIODIC));

	int err = 0;
	static bool initialized;

	__ASSERT_NO_MSG(!initialized);
	initialized = true;

	k_work_init_delayable(&cpu_load_read, cpu_load_read_fn);

	if (IS_ENABLED(CONFIG_DESKTOP_CPU_MEAS_RESULT_NRF_PROFILER)) {
		err = nrf_profiler_init();

		if (!err) {
			register_nrf_profiler_event();
		} else {
			LOG_ERR("nrf_profiler_init failed, err: %d", err);
		}
	}

	if (!err) {
		err = cpu_load_get();

		if (err < 0) {
			LOG_ERR("cpu_load_get failed, err: %d", err);
		} else {
			err = 0;
		}
	}

	if (err) {
		module_set_state(MODULE_STATE_ERROR);
	} else {
		(void)k_work_reschedule(&cpu_load_read, K_MSEC(CONFIG_DESKTOP_CPU_MEAS_PERIOD));
		module_set_state(MODULE_STATE_READY);
	}
}

static bool app_event_handler(const struct app_event_header *aeh)
{
	if (is_module_state_event(aeh)) {
		const struct module_state_event *event = cast_module_state_event(aeh);

		if (check_state(event, MODULE_ID(main), MODULE_STATE_READY)) {
			init();
		}

		return false;
	}

	/* If event is unhandled, unsubscribe. */
	__ASSERT_NO_MSG(false);

	return false;
}

APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);
