#ifndef STATE_FSM
#define STATE_FSM

#include "bsp_uart.h"

typedef enum {
	STATE_INIT,
	STATE_SETF_TEST,
	STATE_IDLE,
	STATE_AUTO_CONTROL,
	STATE_MANUAL_CONTROL,
	STATE_ERROR
}SystemState;

void FSM_init(void);
void FSM_run(void);

void FSM_setState(SystemState state);
SystemState FSM_getState(void);


#endif