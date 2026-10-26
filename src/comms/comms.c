#include "comms.h"
#include "errno.h"
#include "uart/uart.h"

#include <eefs.h>
#include <stdlib.h>
#include <string.h>

#define PACKET_BUFFER_LENGTH (16)
#define BUFFER_FILE_NAME ("__CMD_BUFFER__")

typedef enum comms_state {
  CMMS_WAIT_CMD,
  CMMS_READ_LENGTH,
  CMMS_READ_OP,
  CMMS_READ_DATA,
  CMMS_VALIDATE_PKT,
  CMMS_STORE_DATA_BUFFER,
  CMMS_EXEC_CMMD,
} comms_state;

static comms_state state = CMMS_WAIT_CMD;
static uint8_t data_byte_cout = 0;

static uint16_t curr_op;
static bool reading_op_data = false;

static comms_packet temp_packet = {0};

const eefs_device_t *eefs_dev;
static file_descriptor_t *fs;

void comms_setup(void) {
  eefs_dev = get_eefs_device_handle();
  eefs_dev->api->eefs_open();

  if (eefs_dev->api->eefs_file_exists(BUFFER_FILE_NAME) == ERRNO_SUCCESS) {
    (void)eefs_dev->api->eefs_file_open(fs, BUFFER_FILE_NAME);
  } else {
    (void)eefs_dev->api->eefs_file_create(fs, BUFFER_FILE_NAME, 5096);
  }
}

void comms_update(void) {
  while (uart_data_available()) {
    switch (state) {
    case CMMS_WAIT_CMD: {
      if (uart_data_available())
        state = CMMS_READ_LENGTH;
    } break;

    case CMMS_READ_LENGTH: {
      temp_packet.length = uart_read_byte();
      state = CMMS_READ_OP;
    } break;

    case CMMS_READ_OP: {
      temp_packet.opcode = uart_read_byte();
      state = CMMS_READ_DATA;
    } break;

    case CMMS_READ_DATA: {
      temp_packet.data[data_byte_cout++] = uart_read_byte();
      if (data_byte_cout >= PACKET_DATA_LENGTH) {
        data_byte_cout = 0;
        state = CMMS_VALIDATE_PKT;
      }
    } break;

    case CMMS_VALIDATE_PKT: {
      // check opcode
      switch (temp_packet.opcode) {
      case OPCD_EXEC: {
        curr_op = OPCD_EXEC;
      } break;
      default: {
        curr_op = OPCD_NO_OP;
        // ERROR!
      }
      }

      if (curr_op == OPCD_END_COMMS) {
        state = CMMS_EXEC_CMMD;
        reading_op_data = false;
        break;
      }

      if (curr_op != OPCD_NO_OP) {
        if (!reading_op_data) {
          reading_op_data = true;
        } else {
          eefs_dev->api->eefs_file_append(fs, (uint8_t *)&temp_packet.data,
                                          sizeof(temp_packet.data));
        }
      }

      state = CMMS_READ_LENGTH;
    } break;

    case CMMS_EXEC_CMMD: {
    };
    }
  }
}
