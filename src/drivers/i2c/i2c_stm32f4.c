#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <stdint.h>

#include "errno.h"
#include "i2c/i2c.h"

static inline uint32_t port_to_base_address(const enum i2c_port port) {
  uint32_t addr = UINT32_MAX;

  switch (port) {
  case I2C_PORT_0: {
    addr = I2C1_BASE;
  } break;
  case I2C_PORT_1: {
    addr = I2C2_BASE;
  } break;
  case I2C_PORT_2: {
    addr = I2C3_BASE;
  } break;
  default:
    addr = ERRNO_DRIVER_NO_PORT;
    break;
  }

  return addr;
}

static error_t i2c_stm32_init(struct i2c_controller *controller,
                              const struct i2c_config *config,
                              const enum i2c_port port) {
  error_t err = ERRNO_SUCCESS;

  if (controller == NULL || config == NULL) {
    return -ERRNO_MISC_INVALID_ARG;
  }

  switch (config->speed_hz) {
  case i2c_speed_sm_100k:
    break;
  case i2c_speed_fm_400k:
    break;
  case i2c_speed_fmp_1m:
    break;
  case i2c_speed_unknown:
  default:
#if defined(CONFIG_DRIVERS_DEBUG_ENABLED) && (CONFIG_DRIVERS_DEBUG_ENABLED == 1)
    sys_log_print_event_from_module(SYS_LOG_ERROR, I2C_MODULE_NAME,
                                    "Invalid transfer rate!");
    sys_log_new_line();
#endif        /* CONFIG_DRIVERS_DEBUG_ENABLED */
    err = -1; /* Invalid transfer rate */
    break;
  }

  if (err == 0) {
    uint32_t base_address = port_to_base_address(controller->port);

    /*in both port cases, SDA and SCL will be on port B*/
    rcc_periph_clock_enable(RCC_GPIOB);

    switch (base_address) {
    case I2C1_BASE:
      rcc_periph_clock_enable(RCC_GPIOB);
      rcc_periph_clock_enable(RCC_I2C1);
      rcc_periph_reset_pulse(RST_I2C1);
      gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO6 | GPIO7);

      gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ,
                              GPIO6 | GPIO7);

      gpio_set_af(GPIOB, GPIO_AF4, GPIO6 | GPIO7);
      break;

    case I2C2_BASE:
      rcc_periph_clock_enable(RCC_GPIOB);
      rcc_periph_clock_enable(RCC_I2C2);
      rcc_periph_reset_pulse(RST_I2C2);

      gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11);

      gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ,
                              GPIO10 | GPIO11);

      gpio_set_af(GPIOB, GPIO_AF4, GPIO10 | GPIO11);

      break;

    case I2C3_BASE:
      rcc_periph_clock_enable(RCC_GPIOA);
      rcc_periph_clock_enable(RCC_GPIOB);
      rcc_periph_clock_enable(RCC_I2C3);
      rcc_periph_reset_pulse(RST_I2C3);

      gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8);

      gpio_set_output_options(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO8);

      gpio_set_af(GPIOA, GPIO_AF4, GPIO8);

      gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4);

      gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO4);

      gpio_set_af(GPIOB, GPIO_AF4, GPIO4);
      break;

    default:
#if defined(CONFIG_DRIVERS_DEBUG_ENABLED) && (CONFIG_DRIVERS_DEBUG_ENABLED == 1)
      sys_log_print_event_from_module(SYS_LOG_ERROR, I2C_MODULE_NAME,
                                      "Invalid port!");
      sys_log_new_line();
#endif          /* CONFIG_DRIVERS_DEBUG_ENABLED */
      err = -1; /* Invalid I2C port */
      break;
    }

    if (err == 0) {
      i2c_peripheral_disable(base_address);

      I2C_CR1(base_address) |= I2C_CR1_SWRST;
      I2C_CR1(base_address) &= ~I2C_CR1_SWRST;

      i2c_set_clock_frequency(base_address, config->clock_freq_mhz);
      i2c_set_speed(base_address, config->speed_hz, config->clock_freq_mhz);
      i2c_set_own_7bit_slave_address(base_address, I2C_SLAVE_OWN_7BIT_ADDR);
      i2c_enable_ack(base_address);

      i2c_peripheral_enable(base_address);
    }
  }

  return err;
}

static error_t i2c_stm32_configure(struct i2c_controller *controller,
                                   struct i2c_config *config) {
  if (controller == NULL || config == NULL) {
    return -ERRNO_MISC_INVALID_ARG;
  }

  uint32_t base_address = port_to_base_address(controller->port);

  if (base_address == ERRNO_DRIVER_NO_PORT) {
    return -ERRNO_DRIVER_NO_PORT;
  }

  /*initially disable the peripheral so it can be configured*/
  i2c_peripheral_disable(base_address);

  i2c_set_clock_frequency(base_address, config->clock_freq_mhz);
  i2c_set_speed(base_address, config->speed_hz, config->clock_freq_mhz);

  i2c_peripheral_enable(base_address);

  return ERRNO_SUCCESS;
}

static error_t i2c_stm32_write(struct i2c_device *dev, i2c_slave_adr_t adr,
                               uint8_t *buf, size_t len) {
  error_t err = ERRNO_SUCCESS;

  uint32_t base_address = port_to_base_address(dev->controller->port);

  if (base_address == ERRNO_DRIVER_NO_PORT) {
    return -ERRNO_DRIVER_NO_PORT;
  }

  i2c_transfer7(base_address, adr, buf, len, NULL, 0);
  return err;
}

static error_t i2c_stm32_read(struct i2c_device *dev, i2c_slave_adr_t adr,
                              uint8_t *buf, size_t len) {
  int err = 0;

  uint32_t base_address = port_to_base_address(dev->controller->port);

  if (base_address == ERRNO_DRIVER_NO_PORT) {
    return -ERRNO_DRIVER_NO_PORT;
  }

  i2c_transfer7(base_address, adr, NULL, 0, buf, len);
  return err;
}

static struct i2c_driver_api stm32_i2c_api = {
    .init = i2c_stm32_init,
    .configure = i2c_stm32_configure,
    .write = i2c_stm32_write,
    .read = i2c_stm32_read,
};

static struct i2c_controller stm32_controller_list[] = {
    [I2C_PORT_0] = {0},
    [I2C_PORT_1] = {0},
};

struct i2c_controller *i2c_hw_get_controller_handle(enum i2c_port port) {
  if (port > I2C_PORT_1)
    return NULL;

  return &stm32_controller_list[port];
}

struct i2c_driver_api *i2c_hw_get_driver(void) { return &stm32_i2c_api; }

/** \} End of i2c group */
