#include "board.h"
#include "nrf_drv_gpiote.h"

#define NRF_LOG_MODULE_NAME "GPIO"
#define NRF_LOG_LEVEL 3
#include "nrf_log.h"

static GpioIrqHandler* irq_handlers[32];

static void pin_event_handler(nrf_drv_gpiote_pin_t pin,
			      nrf_gpiote_polarity_t action)
{
	NRF_LOG_DEBUG("int %d %d 0x%x\r\n", pin, action,
		      (uint32_t)irq_handlers[pin]);
	if (irq_handlers[pin]!=NULL)
		irq_handlers[pin]();
}

void GpioInit( Gpio_t *obj, PinNames pin, PinModes mode,  PinConfigs config,
	       PinTypes type, uint32_t value )
{
	ret_code_t err_code;
	obj->pin = pin;
	obj->mode = mode;
	obj->pull = type;
	obj->irq_mode = NO_IRQ;
	NRF_LOG_DEBUG("Init 0x%x %d %d %d\r\n",
		      (uint32_t)obj, pin, mode, type);
	if (mode == PIN_INPUT) {
		nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
		if (type == PIN_PULL_UP)
			config.pull = NRF_GPIO_PIN_PULLUP;
		else if (type == PIN_PULL_DOWN)
			config.pull = NRF_GPIO_PIN_PULLDOWN;
		err_code = nrf_drv_gpiote_in_init(pin, &config, pin_event_handler);
	} else {
		nrf_drv_gpiote_out_config_t config = GPIOTE_CONFIG_OUT_SIMPLE(false);
		if (value!=0)
			config.init_state = true;
		err_code = nrf_drv_gpiote_out_init(pin, &config);
	}
	if (err_code != NRF_SUCCESS) {
		NRF_LOG_ERROR("GpioInit error %d\r\n", err_code);
		return;
	}
	obj->initialized = true;
}

void GpioSetInterrupt( Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority,
		       GpioIrqHandler *irqHandler )
{
	ret_code_t err_code;
	NRF_LOG_DEBUG("SetInterrupt %d %d 0x%x\r\n", obj->pin, irqMode,
		      (uint32_t)irqHandler);

	if (obj->mode != PIN_INPUT) {
		NRF_LOG_ERROR("wrong pin state\r\n");
		return;
	}
	if (obj->initialized) {
		nrf_drv_gpiote_in_uninit(obj->pin);
		obj->initialized = false;
	}

	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);

	obj->irq_mode = irqMode;
	if (irqMode == IRQ_RISING_EDGE) {
		config.sense = NRF_GPIOTE_POLARITY_LOTOHI;
	} else if (irqMode == IRQ_FALLING_EDGE) {
		config.sense = NRF_GPIOTE_POLARITY_HITOLO;
	}

	if (obj->pull == PIN_PULL_UP) {
		config.pull = NRF_GPIO_PIN_PULLUP;
	} else if (obj->pull == PIN_PULL_DOWN) {
		config.pull = NRF_GPIO_PIN_PULLDOWN;
	}

	err_code = nrf_drv_gpiote_in_init(obj->pin, &config, pin_event_handler);
	if (err_code != NRF_SUCCESS) {
		NRF_LOG_ERROR("GpioInit error %d\r\n", err_code);
		return;
	}
	irq_handlers[obj->pin] = irqHandler;
	nrf_drv_gpiote_in_event_enable(obj->pin, true);
	obj->initialized = true;
}

void GpioRemoveInterrupt( Gpio_t *obj )
{
	NRF_LOG_DEBUG("RemoveInterrupt %d\r\n", obj->pin);
	if (obj->mode != PIN_INPUT || obj->initialized != true) {
		NRF_LOG_ERROR("wrong pin state\r\n");
		return;
	}
	nrf_drv_gpiote_in_event_disable(obj->pin);
}

void GpioWrite( Gpio_t *obj, uint32_t value )
{
	NRF_LOG_DEBUG("Write %d %d\r\n", obj->pin, value);
	if (obj->mode != PIN_OUTPUT || obj->initialized != true) {
		NRF_LOG_ERROR("wrong pin state\r\n");
		return;
	}

	if (value) {
		nrf_drv_gpiote_out_set(obj->pin);
	} else {
		nrf_drv_gpiote_out_clear(obj->pin);
	}
}

void GpioToggle( Gpio_t *obj )
{
	NRF_LOG_DEBUG("Toggle %d %d\r\n", obj->pin);
	if (obj->mode != PIN_OUTPUT || obj->initialized != true) {
		NRF_LOG_ERROR("wrong pin state\r\n");
		return;
	}

	nrf_drv_gpiote_out_toggle(obj->pin);
}

uint32_t GpioRead( Gpio_t *obj )
{
	if (obj->mode != PIN_INPUT || obj->initialized != true) {
		NRF_LOG_ERROR("wrong pin state\r\n");
		return 0;
	}
	if (nrf_drv_gpiote_in_is_set(obj->pin))
		return 1;
	else
		return 0;
}

void GpioDeinit( Gpio_t *obj )
{
	NRF_LOG_DEBUG("Deinit %d %d\r\n", obj->pin);
	if (obj->initialized != true) {
		NRF_LOG_DEBUG("wrong pin state\r\n");
		return;
	}
	if (obj->mode == PIN_INPUT) {
		nrf_drv_gpiote_in_uninit(obj->pin);
	} else if (obj->mode == PIN_OUTPUT) {
		nrf_drv_gpiote_out_uninit(obj->pin);
	}
	obj->initialized = false;
}

