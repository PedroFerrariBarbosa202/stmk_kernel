#ifndef stmk_kernel_H_
#define stmk_kernel_H_

#define HANDLERS_COUNT (1)

#define HANDLER_EXEC (0x00)

/* command handlers*/
void exec(void *);

static void (*handlers[HANDLERS_COUNT])(void *) = {
#define OPCODE(cmmd) cmmd,
#include "opcodes.inc"
#undef OPCODE
};

#endif // stmk_kernel_H_
