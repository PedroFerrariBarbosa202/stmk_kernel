#include "flash.h"
#include "errno.h"
#include "libopencm3/stm32/common/flash_common_all.h"
#include "libopencm3/stm32/f4/flash.h"

void flash_unlock_write(void) {
  FLASH_OPTKEYR = FLASH_KEY1;
  FLASH_OPTKEYR = FLASH_KEY2;
}

void flash_clear_sector(uint8_t sector, flash_write_size program_size) {
  flash_erase_sector(sector, (uint32_t)program_size);
}

void flash_mass_erase(flash_write_size size) {
  flash_erase_all_sectors((uint32_t)size);
}

error_t flash_write_byte(const uint32_t addr, const uint8_t data) {
  flash_unlock_write();
  flash_program_byte(addr, data);

  if (FLASH_SR & FLASH_SR_OPERR) {
    return ERRNO_DRIVER_FLASH_OPERR;
  }

  return ERRNO_SUCCESS;
}

error_t flash_write_program(const uint32_t addr_start, const uint8_t *data,
                            const uint32_t length) {
  // remove operation error flag
  // for good measue
  FLASH_SR &= ~FLASH_SR_OPERR;

  flash_unlock_write();

  uint32_t i;
  for (i = 0; i < length; i++) {
    flash_program_byte((addr_start + i), data[i]);

    if (FLASH_SR & FLASH_SR_OPERR) {
      return ERRNO_DRIVER_FLASH_OPERR;
    }
  }

  return ERRNO_SUCCESS;
}
