#ifndef EEFS_EEFS_H
#define EEFS_EEFS_H

#include <stddef.h>
#include <stdint.h>

#include "errno.h"

/* Filesystem configuration */
#define FS_MAGIC 0xCAFEU
#define FILE_MAX_NAME_SIZE 64U
#define MAX_FILES 64U

/* Public file descriptor */
typedef struct file_descriptor_t {
  uint16_t memBlockPtr;

  uint32_t inode;

  uint32_t size;
  uint32_t length;
  uint32_t read_offset;
  uint32_t write_offset;

  uint8_t flags;
  uint8_t permission;
  uint8_t type;
  uint8_t deleted;

  char file_name[FILE_MAX_NAME_SIZE];
} file_descriptor_t;

/* Public filesystem operations */
error_t fs_init(void);
error_t fs_close(void);

error_t file_create(file_descriptor_t *file, const char *file_name,
                    uint32_t size);

error_t file_open(file_descriptor_t *file, const char *name);

error_t file_write(file_descriptor_t *file, const uint8_t *data,
                   uint32_t length);

error_t file_append(file_descriptor_t *file, const uint8_t *data,
                    uint32_t length);

error_t file_read(file_descriptor_t *file, uint8_t *data, uint32_t length);

error_t file_read_chr(file_descriptor_t *file, char *chr);

/* Filesystem device API */
typedef struct eefs_api_t {
  error_t (*eefs_open)(void);
  error_t (*eefs_close)(void);

  error_t (*eefs_file_create)(file_descriptor_t *file, const char *file_name,
                              uint32_t size);

  error_t (*eefs_file_open)(file_descriptor_t *file, const char *file_name);

  error_t (*eefs_file_write)(file_descriptor_t *file, const uint8_t *data,
                             uint32_t length);

  error_t (*eefs_file_append)(file_descriptor_t *file, const uint8_t *data,
                              uint32_t length);

  error_t (*eefs_file_read)(file_descriptor_t *file, uint8_t *data,
                            uint32_t length);

  error_t (*eefs_file_read_chr)(file_descriptor_t *file, char *file_name);
  error_t (*eefs_file_exists)(const char *file_name);
} eefs_api_t;

typedef struct eefs_device_t {
  const eefs_api_t *api;
} eefs_device_t;

/* Retrieve the filesystem device interface */
const eefs_device_t *get_eefs_device_handle(void);

#endif /* EEFS_EEFS_H */
