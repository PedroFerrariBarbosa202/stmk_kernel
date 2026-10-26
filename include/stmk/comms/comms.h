#ifndef comms_H_
#define comms_H_

#include "common-defines.h"

/* packet metadata*/
#define PACKET_LENGTH_BYTES (1)
#define PACKET_OPCD_LENGTH (1)
#define PACKET_DATA_LENGTH (16)
#define PACKET_LENGTH                                                          \
  (PACKET_LENGTH_BYTES + PACKET_DATA_LENGTH + PACKET_OPCD_LENGTH)

/* opcodes*/
#define OPCD_END_COMMS (0x00)
#define OPCD_ACK (0x01)
#define OPCD_NACK (0x02)
#define OPCD_EXEC (0x03)
#define OPCD_NO_OP (0xff)

typedef struct comms_packet {
  uint8_t length;
  uint8_t opcode;
  uint8_t data[PACKET_DATA_LENGTH];
} comms_packet;

void comms_setup(void);
void comms_update(void);

bool comms_packet_available(void);
void comms_write(comms_packet *packet);
void comms_read(comms_packet *packet);

#endif // comms_H_
