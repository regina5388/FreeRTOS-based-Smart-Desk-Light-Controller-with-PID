#include "state_fsm.h"

static SystemState current_state = STATE_ERROR;

void FSM_init(void)
{
	current_state = STATE_INIT;
}

void FSM_run(void)
{
	switch(current_state)
	{
		case STATE_INIT:
			current_state = STATE_IDLE;
			break;
		case STATE_SETF_TEST:
			break;
		case STATE_IDLE:
			break;
		case STATE_AUTO_CONTROL:
			break;
		case STATE_MANUAL_CONTROL:
			break;
		case STATE_ERROR:
			break;
	}
}

void FSM_setState(SystemState state)
{
	current_state = state;
	printf ("state %d is set\r\n", state );
}

SystemState FSM_getState(void)
{
	return current_state;
}