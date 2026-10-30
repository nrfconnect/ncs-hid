/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/types.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/thread_stack.h>
#include <soc.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/reboot.h>

#include <hal/nrf_gpio.h>

#include <caf/key_id.h>
#include <caf/gpio_pins.h>
#include CONFIG_CAF_BUTTONS_DEF_PATH

#include <app_event_manager.h>
#include <caf/events/button_event.h>
#include <caf/events/power_event.h>

#define MODULE buttons
#include <caf/events/module_state_event.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_CAF_BUTTONS_LOG_LEVEL);

#define DEBOUNCE_INTERVAL	CONFIG_CAF_BUTTONS_DEBOUNCE_INTERVAL
#define USED_WORKQ		(IS_ENABLED(CONFIG_CAF_BUTTONS_WQ) ? \
				 (&caf_buttons_wq) : (&k_sys_work_q))

#ifndef CONFIG_CAF_BUTTONS_WQ
/* Unused, but needed to compile the code. */
#define CONFIG_CAF_BUTTONS_WQ_PRIO		0
#define CONFIG_CAF_BUTTONS_WQ_STACK_SIZE	0
#endif

BUILD_ASSERT(ARRAY_SIZE(col) == 0,
	"Low latency mode requires direct GPIO (empty col[] array)");
BUILD_ASSERT(ARRAY_SIZE(row) <= 32,
	"Implementation uses bitmasks (wake_up_mask: uint32_t, "
	"buttons_used_mask: atomic_t) and supports up to 32 buttons");

enum state {
	STATE_DISABLED,
	STATE_INIT,
	STATE_ACTIVE,
	STATE_GOING_TO_OFF,
	STATE_OFF,
	STATE_ERROR
};

enum btn_state {
	BTN_STATE_DISARM,
	BTN_STATE_LISTEN_PRESS,
	BTN_STATE_LISTEN_PRESS_CONFIRMED,
	BTN_STATE_LISTEN_PRESS_WAIT_DEBOUNCE,
	BTN_STATE_LISTEN_RELEASE,
	BTN_STATE_LISTEN_RELEASE_CONFIRMED,
	BTN_STATE_LISTEN_RELEASE_WAIT_DEBOUNCE
};

struct gpio_dev {
	const struct device *const dev;
	const uint8_t port;
	const bool has_gpiote;
	struct gpio_callback gpio_cb;
};

struct btn_ctx {
	struct k_work_delayable debounce;
	atomic_ptr_t button_event;
	atomic_t state;
};

#define DT_DRV_COMPAT nordic_nrf_gpio
#define _GPIO_PORT_VALIDATE(id)							\
	BUILD_ASSERT((DT_INST_PROP(id, port) >= 0) && (DT_INST_PROP(id, port) <= UINT8_MAX));
#define _GPIO_INSTANCE_GET(id)							\
	{									\
		.dev = DEVICE_DT_GET(DT_DRV_INST(id)),				\
		.port = DT_INST_PROP(id, port),					\
		.has_gpiote = DT_INST_NODE_HAS_PROP(id, gpiote_instance),	\
	},

DT_INST_FOREACH_STATUS_OKAY(_GPIO_PORT_VALIDATE)
static struct gpio_dev gpio_devs[] = {
	DT_INST_FOREACH_STATUS_OKAY(_GPIO_INSTANCE_GET)
};
static struct k_spinlock btn_lock;
static struct btn_ctx buttons[ARRAY_SIZE(row)];
static atomic_t buttons_used_mask = ATOMIC_INIT(0);
static uint32_t wake_up_mask;
static atomic_ptr_t button_wake_up_event = ATOMIC_PTR_INIT(NULL);
static atomic_t state = ATOMIC_INIT(STATE_DISABLED);
static atomic_t error_pending = ATOMIC_INIT(false);
static struct k_work error_work;
static atomic_t system_power_off = ATOMIC_INIT(false);

K_THREAD_STACK_DEFINE(caf_buttons_wq_stack_area, CONFIG_CAF_BUTTONS_WQ_STACK_SIZE);
static struct k_work_q caf_buttons_wq;

static void button_isr(const struct device *gpio_dev,
		       struct gpio_callback *cb,
		       uint32_t pins);
static void debounce_fn(struct k_work *work);

static const char *state2str(enum state s)
{
	switch (s) {
	case STATE_DISABLED:
		return "DISABLED";
	case STATE_INIT:
		return "INIT";
	case STATE_ACTIVE:
		return "ACTIVE";
	case STATE_GOING_TO_OFF:
		return "GOING_TO_OFF";
	case STATE_OFF:
		return "OFF";
	case STATE_ERROR:
		return "ERROR";
	default:
		/* Should not happen. */
		__ASSERT_NO_MSG(false);
		return NULL;
	}
}

static void broadcast_module_state(enum state new_state)
{
	static enum module_state last_module_state_sent = MODULE_STATE_COUNT;
	enum module_state module_state_rq = MODULE_STATE_COUNT;

	switch (new_state) {
	case STATE_ACTIVE:
		module_state_rq = MODULE_STATE_READY;
		break;
	case STATE_OFF:
		module_state_rq = MODULE_STATE_STANDBY;
		break;
	case STATE_DISABLED:
	case STATE_ERROR:
		module_state_rq = MODULE_STATE_ERROR;
		break;
	case STATE_GOING_TO_OFF:
	case STATE_INIT:
		return;
	default:
		__ASSERT_NO_MSG(false);
		return;
	}

	if (module_state_rq != last_module_state_sent) {
		last_module_state_sent = module_state_rq;
		module_set_state(module_state_rq);
	}
}

static int get_gpio_idx(uint8_t port)
{
	for (int idx = 0; idx < ARRAY_SIZE(gpio_devs); idx++) {
		if (gpio_devs[idx].port == port) {
			return idx;
		}
	}

	return -EINVAL;
}

static const struct device *get_gpio_dev(uint8_t port)
{
	int idx = get_gpio_idx(port);

	if (idx < 0) {
		return NULL;
	}
	return gpio_devs[idx].dev;
}

static void configuration_validate(void)
{
	for (size_t idx = 0; idx < ARRAY_SIZE(row); idx++) {
		int idy = get_gpio_idx(row[idx].port);

		__ASSERT(idy >= 0,
			 "Button %zu: unknown GPIO port %u",
			 idx, row[idx].port);
		__ASSERT(gpio_devs[idy].has_gpiote,
			 "Button %zu: GPIO port %u has no GPIOTE",
			 idx, row[idx].port);
		__ASSERT(device_is_ready(gpio_devs[idy].dev),
			 "Button %zu: GPIO device not ready", idx);
		ARG_UNUSED(idy);
	}
}

static int configure_inputs(void)
{
	gpio_flags_t flags;
	size_t idx;
	int err;
	const struct device *dev;

	flags = (IS_ENABLED(CONFIG_CAF_BUTTONS_POLARITY_INVERSED) ? GPIO_PULL_UP : GPIO_PULL_DOWN);
	flags |= GPIO_INPUT;
	for (idx = 0; idx < ARRAY_SIZE(row); idx++) {
		dev = get_gpio_dev(row[idx].port);
		__ASSERT(dev != NULL, "Button %zu: unknown GPIO port %u", idx, row[idx].port);
		err = gpio_pin_configure(dev, row[idx].pin, flags);
		if (err) {
			LOG_ERR("Cannot configure button pin: %zu", idx);
			return err;
		}
	}

	return 0;
}

static int configure_gpio_callbacks(void)
{
	size_t idx;
	int err = 0;
	uint32_t pin_mask[ARRAY_SIZE(gpio_devs)] = {0};

	for (idx = 0; idx < ARRAY_SIZE(row); idx++) {
		pin_mask[get_gpio_idx(row[idx].port)] |= BIT(row[idx].pin);
	}
	/* Setup callbacks */
	for (idx = 0; (idx < ARRAY_SIZE(gpio_devs)) && (!err); idx++) {
		gpio_init_callback(&gpio_devs[idx].gpio_cb, button_isr, pin_mask[idx]);
		err = gpio_add_callback(gpio_devs[idx].dev, &gpio_devs[idx].gpio_cb);
	}

	return err;
}

static void init_debounce_works(void)
{
	for (size_t idx = 0; idx < ARRAY_SIZE(row); idx++) {
		k_work_init_delayable(&buttons[idx].debounce, debounce_fn);
	}
}

static gpio_flags_t level_press_irq_flags(void)
{
	return IS_ENABLED(CONFIG_CAF_BUTTONS_POLARITY_INVERSED) ?
			GPIO_INT_LEVEL_LOW : GPIO_INT_LEVEL_HIGH;
}

static gpio_flags_t level_release_irq_flags(void)
{
	return IS_ENABLED(CONFIG_CAF_BUTTONS_POLARITY_INVERSED) ?
			GPIO_INT_LEVEL_HIGH : GPIO_INT_LEVEL_LOW;
}

static int irq_configure(size_t row_idx, gpio_flags_t irq)
{
	const struct device *dev = get_gpio_dev(row[row_idx].port);

	__ASSERT(dev != NULL, "Button %zu: unknown GPIO port %u", row_idx, row[row_idx].port);
	return gpio_pin_interrupt_configure(dev, row[row_idx].pin, irq);
}

static void allocate_button_event(size_t idx, bool pressed)
{
	struct button_event *new_ev;

	if (atomic_ptr_get(&buttons[idx].button_event) != NULL) {
		return;
	}
	new_ev = new_button_event();
	new_ev->key_id = KEY_ID(0, idx);
	new_ev->pressed = pressed;
	if (!atomic_ptr_cas(&buttons[idx].button_event, NULL, new_ev)) {
		app_event_manager_free(new_ev);
	}
}

static void allocate_wakeup_event(void)
{
	struct wake_up_event *new_ev;

	if (atomic_ptr_get(&button_wake_up_event) != NULL) {
		return;
	}
	new_ev = new_wake_up_event();
	if (!atomic_ptr_cas(&button_wake_up_event, NULL, new_ev)) {
		app_event_manager_free(new_ev);
	}
}

static void preallocate_button_press_events(void)
{
	for (size_t idx = 0; idx < ARRAY_SIZE(row); idx++) {
		allocate_button_event(idx, true);
	}
}

static void submit_button_event(size_t idx)
{
	struct button_event *ev =
		(struct button_event *)atomic_ptr_clear(&buttons[idx].button_event);

	if (ev != NULL) {
		APP_EVENT_SUBMIT(ev);
	} else {
		LOG_ERR("Button event for button %zu is not allocated", idx);
	}
}

static void submit_wakeup_event(void)
{
	struct wake_up_event *ev =
		(struct wake_up_event *)atomic_ptr_clear(&button_wake_up_event);

	if (ev != NULL) {
		APP_EVENT_SUBMIT(ev);
	}
}

static int arm_press_listener(size_t row_idx)
{
	atomic_clear_bit(&buttons_used_mask, row_idx);
	(void)atomic_set(&buttons[row_idx].state, BTN_STATE_LISTEN_PRESS);
	return irq_configure(row_idx, level_press_irq_flags());
}

static int arm_release_listener(size_t row_idx)
{
	(void)atomic_set(&buttons[row_idx].state, BTN_STATE_LISTEN_RELEASE);
	return irq_configure(row_idx, level_release_irq_flags());
}

static int arm_release_confirmed_listener(size_t row_idx)
{
	(void)atomic_set(&buttons[row_idx].state, BTN_STATE_LISTEN_RELEASE_CONFIRMED);
	/* GRTC access is needed to schedule a work item after a delay. The operation might take
	 * a longer while right after CPU wakeup from idle. Avoid it while handling a GPIO
	 * interrupt.
	 */
	(void)k_work_reschedule_for_queue(USED_WORKQ, &buttons[row_idx].debounce, K_NO_WAIT);
	return irq_configure(row_idx, GPIO_INT_DISABLE);
}

static int arm_press_confirmed_listener(size_t row_idx)
{
	atomic_set_bit(&buttons_used_mask, row_idx);
	(void)atomic_set(&buttons[row_idx].state, BTN_STATE_LISTEN_PRESS_CONFIRMED);
	/* GRTC access is needed to schedule a work item after a delay. The operation might take
	 * a longer while right after CPU wakeup from idle. Avoid it while handling a GPIO
	 * interrupt.
	 */
	(void)k_work_reschedule_for_queue(USED_WORKQ, &buttons[row_idx].debounce, K_NO_WAIT);
	return irq_configure(row_idx, GPIO_INT_DISABLE);
}

static int disarm_listener(size_t row_idx)
{
	(void)atomic_set(&buttons[row_idx].state, BTN_STATE_DISARM);
	return irq_configure(row_idx, GPIO_INT_DISABLE);
}

static void deactivate_listeners(void)
{
	for (size_t idx = 0; idx < ARRAY_SIZE(row); idx++) {
		if (disarm_listener(idx)) {
			LOG_ERR("Cannot disable IRQ for the button: %zu", idx);
		}
	}
	(void)atomic_clear(&buttons_used_mask);
}

static uint32_t init_wakeup_mask(const struct gpio_pin *pins, size_t cnt)
{
	uint32_t mask = BIT64_MASK(32);

	for (size_t idx = 0; idx < cnt; idx++) {
		WRITE_BIT(mask, idx, !pins[idx].wakeup_blocked);
	}

	return mask;
}

static int setup_pin_wakeup(void)
{
	int err = 0;

	for (size_t idx = 0; (idx < ARRAY_SIZE(row)) && (!err); idx++) {
		if (wake_up_mask & BIT(idx)) {
			err = arm_press_listener(idx);
		} else {
			err = disarm_listener(idx);
		}
	}

	return err;
}

static void cancel_debounce_buttons_works(void)
{
	for (size_t idx = 0; idx < ARRAY_SIZE(row); idx++) {
		(void)k_work_cancel_delayable(&buttons[idx].debounce);
	}
}

static void go_to_error_state_from_isr(void)
{
	if (atomic_get(&state) == STATE_ERROR) {
		return;
	}
	deactivate_listeners();
	cancel_debounce_buttons_works();
	if (!atomic_cas(&error_pending, false, true)) {
		return;
	}
	/* Deferred transition by error_work ensures that the state
	 * will not be overwritten by another context.
	 */
	(void)k_work_submit_to_queue(USED_WORKQ, &error_work);
}

static void update_state(enum state new_state)
{
	if ((atomic_get(&state) == STATE_ERROR) ||
		(atomic_get(&state) == new_state)) {
		return;
	}
	if (new_state == STATE_ERROR) {
		deactivate_listeners();
		cancel_debounce_buttons_works();
	}
	if (atomic_get(&error_pending) && (new_state != STATE_ERROR)) {
		return;
	}
	(void)atomic_set(&state, new_state);
	LOG_INF("Entering state: %s", state2str(new_state));
	broadcast_module_state(new_state);
}

static void error_work_fn(struct k_work *work)
{
	ARG_UNUSED(work);

	update_state(STATE_ERROR);
}

#if defined(NRF_GPIO_LATCH_PRESENT)
static void reset_button_pin_hw(size_t row_idx)
{
	uint32_t abs_pin = NRF_GPIO_PIN_MAP(row[row_idx].port, row[row_idx].pin);

	/* After system OFF, the GPIO LATCH bit for a pin may still be set.
	 * LATCH is retained through power OFF and reboot. Clearing LATCH
	 * prevents spurious GPIO interrupts right after waking up from system OFF.
	 */
	nrf_gpio_pin_latch_clear(abs_pin);
}
#endif

static __maybe_unused bool is_pin_pressed(size_t row_idx)
{
	uint32_t abs_pin = NRF_GPIO_PIN_MAP(row[row_idx].port, row[row_idx].pin);
	int val = nrf_gpio_pin_read(abs_pin);

	if (IS_ENABLED(CONFIG_CAF_BUTTONS_POLARITY_INVERSED)) {
		val = !val;
	}

	return val != 0;
}

static int activate_listeners(void)
{
	int err = 0;

	for (size_t idx = 0; idx < ARRAY_SIZE(row); idx++) {
#if defined(NRF_GPIO_LATCH_PRESENT)
		reset_button_pin_hw(idx);
#endif
		err = arm_press_listener(idx);
		if (err != 0) {
			break;
		}
	}

	return err;
}

static void button_isr_in_off_state_handle(size_t dev_idx, uint32_t pins)
{
	size_t idx;

	for (idx = 0; idx < ARRAY_SIZE(row); idx++) {
		if (get_gpio_idx(row[idx].port) != dev_idx) {
			continue;
		}
		if (!(pins & BIT(row[idx].pin))) {
			continue;
		}
		if (irq_configure(idx, GPIO_INT_DISABLE)) {
			go_to_error_state_from_isr();
			return;
		}
	}
	if (IS_ENABLED(CONFIG_CAF_BUTTONS_PM_EVENTS)) {
		submit_wakeup_event();
	}
}

static void button_isr_in_active_state_handle(size_t dev_idx, uint32_t pins)
{
	size_t idx;

	for (idx = 0; idx < ARRAY_SIZE(row); idx++) {
		if (get_gpio_idx(row[idx].port) != dev_idx) {
			continue;
		}
		if (!(pins & BIT(row[idx].pin))) {
			continue;
		}
		switch (atomic_get(&buttons[idx].state)) {
		case BTN_STATE_LISTEN_PRESS:
			if (IS_ENABLED(CONFIG_CAF_BUTTONS_NRF54LS05_SPURIOUS_PRESSES_WORKAROUND) &&
			    !is_pin_pressed(idx)) {
				break;
			}
			if (arm_press_confirmed_listener(idx)) {
				go_to_error_state_from_isr();
			} else {
				submit_button_event(idx);
			}
			break;
		case BTN_STATE_LISTEN_RELEASE:
			if (IS_ENABLED(CONFIG_CAF_BUTTONS_NRF54LS05_SPURIOUS_PRESSES_WORKAROUND) &&
			    is_pin_pressed(idx)) {
				break;
			}
			if (arm_release_confirmed_listener(idx)) {
				go_to_error_state_from_isr();
			} else {
				submit_button_event(idx);
			}
			break;
		default:
			LOG_ERR("Unwanted ISR/IRQ for the button: %zu", idx);
			break;
		}
	}
}

static void button_isr(const struct device *gpio_dev,
		       struct gpio_callback *cb,
		       uint32_t pins)
{
	ARG_UNUSED(gpio_dev);
	if (IS_ENABLED(CONFIG_CAF_BUTTONS_PM_EVENTS) && atomic_get(&system_power_off)) {
		/* Instantly reboot the system to prevent system power off with GPIO */
		/* interrupts disabled. */
		sys_reboot(SYS_REBOOT_WARM);
		return;
	}
	struct gpio_dev *gpio = CONTAINER_OF(cb, struct gpio_dev, gpio_cb);
	size_t dev_idx = ARRAY_INDEX(gpio_devs, gpio);

	__ASSERT_NO_MSG(dev_idx >= 0);
	switch (atomic_get(&state)) {
	case STATE_OFF:
		button_isr_in_off_state_handle(dev_idx, pins);
		break;
	case STATE_ACTIVE:
	case STATE_GOING_TO_OFF:
		button_isr_in_active_state_handle(dev_idx, pins);
		break;
	default:
		break;
	}
}

static void debounce_fn(struct k_work *work)
{
	struct k_work_delayable *dwork = k_work_delayable_from_work(work);
	struct btn_ctx *btn = CONTAINER_OF(dwork, struct btn_ctx, debounce);
	size_t row_idx = ARRAY_INDEX(buttons, btn);
	enum btn_state btn_state = atomic_get(&buttons[row_idx].state);
	int err;

	__ASSERT_NO_MSG(
		(atomic_get(&state) == STATE_ACTIVE) ||
		(atomic_get(&state) == STATE_GOING_TO_OFF));

	switch (btn_state) {
	case BTN_STATE_LISTEN_PRESS_CONFIRMED:
	case BTN_STATE_LISTEN_RELEASE_CONFIRMED:
		/* Delay GPIO interrupt reenable (button debouncing). */
		enum btn_state new_btn_state = (btn_state == BTN_STATE_LISTEN_PRESS_CONFIRMED) ?
						BTN_STATE_LISTEN_PRESS_WAIT_DEBOUNCE :
						BTN_STATE_LISTEN_RELEASE_WAIT_DEBOUNCE;

		(void)atomic_set(&buttons[row_idx].state, new_btn_state);
		(void)k_work_reschedule_for_queue(USED_WORKQ, dwork, K_MSEC(DEBOUNCE_INTERVAL));
		/* Execution will be continued after the delay. Early return. */
		return;

	case BTN_STATE_LISTEN_PRESS_WAIT_DEBOUNCE:
		allocate_button_event(row_idx, false);
		err = arm_release_listener(row_idx);
		if (err) {
			LOG_ERR("Cannot arm release listener (err: %d)", err);
			update_state(STATE_ERROR);
			return;
		}
		break;

	case BTN_STATE_LISTEN_RELEASE_WAIT_DEBOUNCE:
		allocate_button_event(row_idx, true);
		err = arm_press_listener(row_idx);
		if (err) {
			LOG_ERR("Cannot arm press listener (err: %d)", err);
			update_state(STATE_ERROR);
			return;
		}
		break;

	default:
		/* Should not happen. */
		__ASSERT_NO_MSG(false);
		update_state(STATE_ERROR);
		return;
	}

	if (atomic_get(&state) == STATE_GOING_TO_OFF) {
		k_spinlock_key_t key = k_spin_lock(&btn_lock);
		/* Wake-up event is already allocated */

		if (!atomic_get(&buttons_used_mask)) {
			if (setup_pin_wakeup()) {
				update_state(STATE_ERROR);
			} else {
				update_state(STATE_OFF);
			}
		}
		k_spin_unlock(&btn_lock, key);
	}
}

static int suspend(void)
{
	int err = 0;

	switch (atomic_get(&state)) {
	case STATE_ACTIVE: {
		if (IS_ENABLED(CONFIG_CAF_BUTTONS_PM_EVENTS)) {
			/* Allocate wake-up event even if buttons are used
			 * to reduce time in IRQ lock.
			 */
			allocate_wakeup_event();
		}
		k_spinlock_key_t key = k_spin_lock(&btn_lock);

		if (!atomic_get(&buttons_used_mask)) {
			/* No buttons in use */
			err = setup_pin_wakeup();
			if (err) {
				update_state(STATE_ERROR);
			} else {
				update_state(STATE_OFF);
			}
		} else {
			update_state(STATE_GOING_TO_OFF);
			err = -EACCES;
		}
		k_spin_unlock(&btn_lock, key);
		break;
	}
	case STATE_GOING_TO_OFF:
		err = -EACCES;
		break;
	case STATE_OFF:
		err = -EALREADY;
		break;
	default:
		err = -ENOTSUP;
		break;
	}

	return err;
}

static void resume(void)
{
	int err;

	switch (atomic_get(&state)) {
	case STATE_OFF:
		deactivate_listeners();
		preallocate_button_press_events();
		update_state(STATE_ACTIVE);
		err = activate_listeners();
		if (err) {
			LOG_ERR("Cannot activate listeners (err: %d)", err);
			update_state(STATE_ERROR);
		}
		break;
	case STATE_GOING_TO_OFF:
		update_state(STATE_ACTIVE);
		break;
	default:
		break;
	}
}

static void init_caf_buttons_wq(void)
{
	static const struct k_work_queue_config cfg = {.name = "CAF Buttons WQ"};

	k_work_queue_init(&caf_buttons_wq);
	k_work_queue_start(&caf_buttons_wq, caf_buttons_wq_stack_area,
			   K_THREAD_STACK_SIZEOF(caf_buttons_wq_stack_area),
			   CONFIG_CAF_BUTTONS_WQ_PRIO, &cfg);
}

static void init_fn(void)
{
	update_state(STATE_INIT);
	/* Check configuration */
	configuration_validate();
	/* Configure release buttons works */
	init_debounce_works();
	/* Preallocate button press events */
	preallocate_button_press_events();

	if (IS_ENABLED(CONFIG_CAF_BUTTONS_WQ)) {
		init_caf_buttons_wq();
	}

	/* Configure pins as inputs */
	int err = configure_inputs();

	if (err) {
		LOG_ERR("Cannot configure inputs (err: %d)", err);
		update_state(STATE_ERROR);
		return;
	}
	/* Disable GPIO interrupts */
	deactivate_listeners();
	/* Prepare wake-up bit mask */
	wake_up_mask = init_wakeup_mask(row, ARRAY_SIZE(row));
	/* Configure GPIO interrupt callbacks */
	err = configure_gpio_callbacks();
	if (err) {
		LOG_ERR("Cannot add callback (err: %d)", err);
		update_state(STATE_ERROR);
		return;
	}
	/* Initialize buttons state */
	/* Activate module */
	update_state(STATE_ACTIVE);
	err = activate_listeners();
	if (err) {
		LOG_ERR("Cannot activate listeners (err: %d)", err);
		update_state(STATE_ERROR);
		return;
	}
}

static bool app_event_handler(const struct app_event_header *aeh)
{
	if (is_module_state_event(aeh)) {
		const struct module_state_event *event = cast_module_state_event(aeh);

		if (check_state(event, MODULE_ID(main), MODULE_STATE_READY)) {
			__ASSERT_NO_MSG(atomic_get(&state) == STATE_DISABLED);
			k_work_init(&error_work, error_work_fn);
			init_fn();
		}
		return false;
	}
	if (IS_ENABLED(CONFIG_CAF_BUTTONS_PM_EVENTS) &&
		is_wake_up_event(aeh)) {
		resume();
		return false;
	}
	if (IS_ENABLED(CONFIG_CAF_BUTTONS_PM_EVENTS) &&
		is_power_down_event(aeh)) {
		int err = suspend();

		if (!err) {
			return false;
		} else if (-EALREADY == err) {
			/* Module already suspended */
			return false;
		} else if (-EACCES == err) {
			/* Consume event in case of situation when button is used */
			/* Button already pressed or waiting for debounce timeout */
			return true;
		}
		update_state(STATE_ERROR);
		return false;
	}
	if (IS_ENABLED(CONFIG_CAF_BUTTONS_PM_EVENTS) &&
		is_power_off_event(aeh)) {
		(void)atomic_set(&system_power_off, true);

		/* Check if an attempt to wake up the system has already been made.
		 * If the system is about to go into power off, the only
		 * way to wake it up is by issuing a warm reboot.
		 *
		 * Checking the button_wake_up_event pointer is needed to avoid a race
		 * condition where the button_isr preempts this function before
		 * system_power_off is set.
		 */
		if (atomic_get(&state) != STATE_OFF ||
		    atomic_ptr_get(&button_wake_up_event) == NULL) {
			/* Instantly reboot the system to prevent system power off with GPIO */
			/* interrupts disabled. */
			sys_reboot(SYS_REBOOT_WARM);
		}
		return false;
	}

	__ASSERT_NO_MSG(false);
	return false;
}

APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);
#if CONFIG_CAF_BUTTONS_PM_EVENTS
APP_EVENT_SUBSCRIBE_EARLY(MODULE, power_down_event);
APP_EVENT_SUBSCRIBE(MODULE, power_off_event);
APP_EVENT_SUBSCRIBE(MODULE, wake_up_event);
#endif
