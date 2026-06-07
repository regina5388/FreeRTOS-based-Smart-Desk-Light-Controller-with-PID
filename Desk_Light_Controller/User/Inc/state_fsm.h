#ifndef STATE_FSM
#define STATE_FSM

#include "bsp_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "timers.h"
#include "bsp_rot_encoder.h"

#define TIMER_DEBUG 1

#if (TIMER_DEBUG == 1)
extern TickType_t focus_start_tick;
extern TickType_t break_start_tick;
extern TickType_t focus_timeout_tick;
extern TickType_t break_timeout_tick;
#endif

#define INTIALIZE_DONE_EVENT 	(0x01 << 0)
#define BUTTON_PRESSED_EVENT	(0x01 << 1)
#define SENSOR_UPDATE_EVENT 	(0x01 << 2)
#define ENCODER_ROTATE_EVENT 	(0x01 << 3)
#define FOCUS_TIMEOUT_EVENT 	(0x01 << 4)
#define BREAK_TIMEOUT_EVENT 	(0x01 << 5)

typedef enum {
	STATE_INIT,
	STATE_SETF_TEST,
	STATE_IDLE,
	STATE_AUTO_CONTROL,
	STATE_MANUAL_CONTROL,
	STATE_FOCUS,
	STATE_BREAK,
	STATE_ERROR
}SystemState;

void FSM_init(void);
void FSM_run(EventBits_t r_event);

void FSM_setState(SystemState state);
SystemState FSM_getState(void);


#endif