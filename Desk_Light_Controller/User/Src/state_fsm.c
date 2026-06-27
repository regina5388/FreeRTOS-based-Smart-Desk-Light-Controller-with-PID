#include "state_fsm.h"

static SystemState current_state = STATE_ERROR;

#define LUX_MIN    50.0f
#define LUX_MAX    80.0f

static float current_lux = 0;
static float target_lux = 500;

static int16_t manual_brightness = 0;   // user-set brightness
static int16_t current_brightness = 0;   // actual commanded PWM 0~100

static int16_t encoder_delta = 0;

extern SemaphoreHandle_t UART_MuxSem_Handle;
extern QueueHandle_t Brightness_Queue_Handle;
extern QueueHandle_t Encoder_Delta_Queue_Handle;
extern QueueHandle_t Lux_Queue_Handle;

extern TimerHandle_t FocusTimer_SWTimer_Handle;
extern TimerHandle_t BreakTimer_SWTimer_Handle;


	
void FSM_init(void)
{
	current_state = STATE_INIT;
}

void FSM_run(EventBits_t r_event)
{
	BaseType_t xReturn = pdFALSE;
	
	switch(current_state)
	{
		case STATE_INIT:
			current_state = STATE_IDLE;
			break;
		case STATE_SETF_TEST:
			break;
		case STATE_IDLE:
			if (r_event & BUTTON_PRESSED_EVENT)
			{
				current_state = STATE_AUTO_CONTROL;
			}
			
			xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
			printf("IDLE ! \r\n");
			xSemaphoreGive(UART_MuxSem_Handle);
			break;
		case STATE_AUTO_CONTROL:
			//receive lux from sensor, Call pid, send queue of PWM to LED
			
			if (r_event & BUTTON_PRESSED_EVENT)
			{
				
				current_state = STATE_MANUAL_CONTROL;
				manual_brightness = current_brightness;
			}
			
			if (r_event & SENSOR_UPDATE_EVENT)
			{
				xReturn = xQueueReceive( Lux_Queue_Handle,
									&current_lux,
									0 );
				
				xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
				printf("Auto ! lux measured is  %f\r\n", current_lux);
				xSemaphoreGive(UART_MuxSem_Handle);
			}
		
			xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
			printf("Auto ! \r\n");
			xSemaphoreGive(UART_MuxSem_Handle);
			
			if(xReturn == pdTRUE)
			{
				/************lux measured is only 0-80, 80/65535 too small************/
				current_brightness = 50;
				
				
				
//				if (current_brightness > LUX_MAX)
//				{
//					current_brightness = 100;
//				}

//				if (current_brightness < LUX_MIN)
//				{
//					current_brightness = 0;
//				}
					//***********current_breghtness = pid(current_lux)***********//
					xReturn = xQueueOverwrite( Brightness_Queue_Handle,
									&current_brightness);
			}
			

			
			break;
		case STATE_MANUAL_CONTROL:
			//receive delta from encoder, calculate brightness, send queue of PWM to LED
		
			if (r_event & BUTTON_PRESSED_EVENT)
			{
				current_state = STATE_FOCUS;
				current_brightness = manual_brightness;
				
				focus_start_tick = xTaskGetTickCount();
				xTimerReset(FocusTimer_SWTimer_Handle, 0);
				
				xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
				printf("Enter FOCUS, tick = %d\r\n", focus_start_tick);
				xSemaphoreGive(UART_MuxSem_Handle);
				
				
			}
		
			if (r_event & ENCODER_ROTATE_EVENT)
			{
				xReturn = xQueueReceive( Encoder_Delta_Queue_Handle,
									&encoder_delta,
									0 );
				if(xReturn == pdTRUE)
				{
					manual_brightness += encoder_delta * STEP;
		
					if (manual_brightness > 100)
						manual_brightness = 100;
					
					if (manual_brightness < 0)
						manual_brightness = 0;
					
					current_brightness = manual_brightness;
					
						xReturn = xQueueOverwrite( Brightness_Queue_Handle,
										&current_brightness);
				}
				
				xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
				printf("delta=%d\r\n", encoder_delta);
				xSemaphoreGive(UART_MuxSem_Handle);
			}
			
			break;
		case STATE_FOCUS:
			//Add pid logic afterwards?
			
			
		
			if (r_event & BUTTON_PRESSED_EVENT)
			{
				current_state = STATE_AUTO_CONTROL;
				xTimerStop(FocusTimer_SWTimer_Handle, 0);
			}
			
				xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
				printf("FOCUS!\r\n");
				xSemaphoreGive(UART_MuxSem_Handle);
			
			if (r_event & FOCUS_TIMEOUT_EVENT)
			{	
				
				current_state = STATE_BREAK;
				
				break_start_tick = xTaskGetTickCount();
				
				xTimerStop(FocusTimer_SWTimer_Handle, 0);
				xTimerReset(BreakTimer_SWTimer_Handle, 0);
				
				xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
				printf("Enter BREAK, tick = %d\r\n", break_start_tick);
				xSemaphoreGive(UART_MuxSem_Handle);
				//Blink Lights for hints
			}
		
			break;
		
		case STATE_BREAK:
			//Add pid logic afterwards?
			if (r_event & BUTTON_PRESSED_EVENT)
			{
				current_state = STATE_AUTO_CONTROL;
				xTimerStop(BreakTimer_SWTimer_Handle, 0);

			}
			
				xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
				printf("BREAK!\r\n");
				xSemaphoreGive(UART_MuxSem_Handle);
			
			if (r_event & BREAK_TIMEOUT_EVENT)
			{
				focus_start_tick = xTaskGetTickCount();
				
				current_state = STATE_FOCUS;
				
				xTimerReset(FocusTimer_SWTimer_Handle, 0);
				xTimerStop(BreakTimer_SWTimer_Handle, 0);
				
				xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
				printf("Enter FOCUS, tick = %d\r\n", focus_start_tick);
				xSemaphoreGive(UART_MuxSem_Handle);
				
				//Blink Lights for hints
			}
		
			break;
		case STATE_ERROR:
			
			xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
			printf("Error! \r\n");
			xSemaphoreGive(UART_MuxSem_Handle);
			
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

