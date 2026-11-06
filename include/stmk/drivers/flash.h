#ifndef flash_H_
#define flash_H_

#include "errno.h"
#include "stdint.h"

#define FLASH_KEY1 (0x08192A3B)
#define FLASH_KEY2 (0x4C5D6E7F)

typedef enum flash_write_size {
  flash_byte = 0,
  flash_half_word,
  flash_word,
  flash_double_word,
} flash_write_size;

void flash_clear_sector(uint8_t sector, flash_write_size program_size);
void flash_mass_erase(flash_write_size size);

error_t flash_write_byte(const uint32_t addr, const uint8_t data);
error_t flash_write_program(const uint32_t addr_start, const uint8_t *data,
                            const uint32_t length);

#endif // flash_H_
