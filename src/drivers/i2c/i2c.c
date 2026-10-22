#include <stdint.h>

#include "errno.h"
#include "i2c/i2c.h"

error_t i2c_init_controller(struct i2c_controller **controller,
                            const enum i2c_port port,
                            const struct i2c_config *config) {
  struct i2c_driver_api *api;
  struct i2c_controller *ctrl;
  int err;

  if (!controller)
    return -ERRNO_MISC_INVALID_ARG;

  ctrl = i2c_hw_get_controller_handle(port);

  if (ctrl == NULL)
    return -ERRNO_DRIVER_NO_PORT;

  *controller = ctrl;

  if (ctrl->initialized == 1)
    return ERRNO_SUCCESS;

  /* This check is in here to enable config to be NULL, if the controller
   * is already be initialized. */
  if (!config)
    return -ERRNO_MISC_INVALID_ARG;

  api = i2c_hw_get_driver();

  if (!api)
    return -ERRNO_MISC_UNSUPPORTED_OP;

  if (err < 0)
    return err;

  err = api->init(ctrl, config, port);

  if (err < 0)
    return err;

  (*controller)->port = port;
  (*controller)->api = *api;
  (*controller)->config = *config;
  (*controller)->initialized = 1U;

  return 0;
}

error_t i2c_configure_controller(struct i2c_controller *controller,
                                 struct i2c_config *config) {
  error_t err = ERRNO_SUCCESS;

  if (!config || !controller)
    return -ERRNO_MISC_INVALID_ARG;

  if (controller->initialized != 1U)
    return -ERRNO_DRIVER_UNINITIALIZED;

  err = controller->api.configure(controller, config);

  if (err < 0)
    return err;

  controller->config = *config;

  return 0;
}

error_t i2c_init_device(struct i2c_device *dev, enum i2c_port port,
                        const struct i2c_config *config) {
  error_t err = ERRNO_SUCCESS;

  if (!dev)
    return -ERRNO_MISC_INVALID_ARG;

  err = i2c_init_controller(&dev->controller, port, config);

  if (err < 0)
    return err;

  return 0;
}

error_t i2c_device_write(struct i2c_device *dev, i2c_slave_adr_t addr,
                         uint8_t *buf, size_t len) {
  struct i2c_controller *controller = dev->controller;
  error_t err = ERRNO_SUCCESS;

  if (!dev || !dev->controller)
    return -ERRNO_MISC_INVALID_ARG;

  if (err < 0)
    return err;

  err = controller->api.write(dev, addr, buf, len);

  return err;
}

error_t i2c_device_read(struct i2c_device *dev, i2c_slave_adr_t addr,
                        uint8_t *buf, size_t len) {
  struct i2c_controller *controller = dev->controller;
  error_t err = ERRNO_SUCCESS;

  if (!dev || !dev->controller)
    return -ERRNO_MISC_INVALID_ARG;

  if (err < 0)
    return err;

  err = controller->api.read(dev, addr, buf, len);

  return err;
}

/** \} End of spi group */
