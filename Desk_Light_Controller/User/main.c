#include "bsp_initialize.h"

#define QUEUE_LEN 		(1)

#define FOCUS_TIME_MS 	(25UL * 1000UL)
#define BREAK_TIME_MS 	(5UL * 1000UL)


static StackType_t Idle_Task_Stack[configMINIMAL_STACK_SIZE];
static StackType_t Timer_Task_Stack[configTIMER_TASK_STACK_DEPTH];
static StackType_t LED_Task_Stack[configTIMER_TASK_STACK_DEPTH];
static StackType_t Sensor_Task_Stack[configTIMER_TASK_STACK_DEPTH];
static StackType_t Button_Task_Stack[configTIMER_TASK_STACK_DEPTH];
static StackType_t Encoder_Task_Stack[configTIMER_TASK_STACK_DEPTH];
static StackType_t FSM_Task_Stack[configTIMER_TASK_STACK_DEPTH];

static StaticTask_t Idle_Task_TCB;
static StaticTask_t Timer_Task_TCB;
static StaticTask_t LED_Task_TCB;
static StaticTask_t Sensor_Task_TCB;
static StaticTask_t Button_Task_TCB;
static StaticTask_t Encoder_Task_TCB;
static StaticTask_t FSM_Task_TCB;

static StaticTimer_t Focus_Timer_TCB;
static StaticTimer_t Break_Timer_TCB;

static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t LED_Task_Handle		 = NULL;
static TaskHandle_t Sensor_Task_Handle	 = NULL;
static TaskHandle_t Button_Task_Handle	 = NULL;
static TaskHandle_t FSM_Task_Handle	 	 = NULL;
static TaskHandle_t Encoder_Task_Handle	 = NULL;

 TimerHandle_t FocusTimer_SWTimer_Handle = NULL;
 TimerHandle_t BreakTimer_SWTimer_Handle = NULL;
 
 QueueHandle_t Brightness_Queue_Handle = NULL;
 QueueHandle_t Encoder_Delta_Queue_Handle = NULL;
 QueueHandle_t Lux_Queue_Handle = NULL;

SemaphoreHandle_t Button_BiSem_Handle = NULL;
SemaphoreHandle_t UART_MuxSem_Handle = NULL;

static EventGroupHandle_t FSM_Event_Handle = NULL;

#if (TIMER_DEBUG == 1)
TickType_t focus_start_tick = 0;
TickType_t break_start_tick = 0;
TickType_t focus_timeout_tick = 0;
TickType_t break_timeout_tick = 0;
#endif


void Delay(__IO uint32_t nCount);
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize);

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
									StackType_t **ppxIdleTaskStackBuffer,
									uint32_t *pulIdleTaskStackSize);

static void LED_Task(void* parameter);
static void Sensor_Task(void* parameter);
static void Button_Task(void* parameter);
static void Encoder_Task(void* parameter);
static void FSM_Task(void* parameter);
static void AppTaskCreate_Task(void* parameter);
static void FocusTimer_SWTimer_Callback(void *parameter);
static void BreakTimer_SWTimer_Callback(void *parameter);
	
int main(void)
{
	BaseType_t xReturn = pdPASS;
	
	BSP_Init();

    printf("TEST starts now\r\n");
	
	xReturn = xTaskCreate( AppTaskCreate_Task,
										(const char*) "AppTaskCreate",
										(u32)configMINIMAL_STACK_SIZE,
										(void*) NULL,
										(UBaseType_t)3,
										(TaskHandle_t)&AppTaskCreate_Handle);
	if (xReturn == pdPASS)
	{	
		vTaskStartScheduler();
	}

    for (;;);
}

static void FocusTimer_SWTimer_Callback(void *parameter)
{
	focus_timeout_tick = xTaskGetTickCount();
	xEventGroupSetBits(FSM_Event_Handle, FOCUS_TIMEOUT_EVENT);
	
	xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
	printf("FOCUS timeout, tick = %d, elapsed = %d ms\r\n",
           focus_timeout_tick,
           (focus_timeout_tick - focus_start_tick) * portTICK_PERIOD_MS);
	xSemaphoreGive(UART_MuxSem_Handle);
}

static void BreakTimer_SWTimer_Callback(void *parameter)
{
	break_timeout_tick = xTaskGetTickCount();
	xEventGroupSetBits(FSM_Event_Handle, BREAK_TIMEOUT_EVENT);
	
	xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
	printf("BREAK timeout, tick = %d, elapsed = %d ms\r\n",
	  break_timeout_tick,
	   (break_timeout_tick - break_start_tick) * portTICK_PERIOD_MS);
	xSemaphoreGive(UART_MuxSem_Handle);
}

static void LED_Task(void* parameter)
{
	BaseType_t xReturn = NULL;
	int16_t Brightness_buf;
	while(1)
	{
		xReturn = xQueueReceive( Brightness_Queue_Handle,
							  &Brightness_buf,
							  0);
		if(xReturn != NULL)
		{
			xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
			printf("Receive Brightness ! \r\n");
			xSemaphoreGive(UART_MuxSem_Handle);
		}
		
		Set_DutyCycle_LED(Brightness_buf);
		vTaskDelay(pdMS_TO_TICKS(20));
	}
	
}

static void Sensor_Task(void* parameter)
{
	BaseType_t xReturn = pdFAIL;
	
	float lux = 0.0f;
	uint8_t sensor_on = 0;
	
	while(1)
	{
		if(FSM_getState() == STATE_AUTO_CONTROL)
		{
			if(sensor_on == 0)
			{
				LIGHT_SENSOR_Init();
				vTaskDelay(pdMS_TO_TICKS(180));
				sensor_on = 1;
			}
			
			if(LIGHT_SENSOR_ReadLux(&lux)== 1) 
			{
				xReturn = xQueueOverwrite( Lux_Queue_Handle,
								  &lux);
				
				if(xReturn == pdPASS)
				{
					xEventGroupSetBits(FSM_Event_Handle, SENSOR_UPDATE_EVENT);
					
					xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
					printf("Sent lux: %d lx\r\n", (uint16_t)lux);
					xSemaphoreGive(UART_MuxSem_Handle);
				}
			
			}
			vTaskDelay(pdMS_TO_TICKS(180));		
		}
		
		else
		{
			if(sensor_on == 1)
			{
				LIGHT_SENSOR_PowerDown();
				sensor_on = 0;
			}
			 vTaskDelay(pdMS_TO_TICKS(100));
		}
		
	}
	
}


static void Button_Task(void* parameter)
{
	BaseType_t xReturn = pdTRUE;
	
	while(1)
	{
		// Wait for button interrupt semaphore
		xReturn = xSemaphoreTake(Button_BiSem_Handle, portMAX_DELAY);
		
		if (xReturn == pdTRUE)
		{
			// Debounce press
			vTaskDelay(pdMS_TO_TICKS(20));
			
			// Confirm button is still pressed
			if (GPIO_ReadInputDataBit(ENCODER_KEY_GPIO_PORT, ENCODER_KEY_GPIO_PIN) == BUTTON_ON)
			{
				
				// Wait for button release
				while (GPIO_ReadInputDataBit(ENCODER_KEY_GPIO_PORT, ENCODER_KEY_GPIO_PIN) == BUTTON_ON)
				{
					vTaskDelay(pdMS_TO_TICKS(5));
				}
				
				// Debounce release
				vTaskDelay(pdMS_TO_TICKS(20));

				// Release detected
				xEventGroupSetBits(FSM_Event_Handle, BUTTON_PRESSED_EVENT);
				
			}
		}
		else
		{
			// Should rarely happen with portMAX_DELAY
			xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
			printf("receive error, error code 0x%1x\n", (uint32_t)xReturn);
			xSemaphoreGive(UART_MuxSem_Handle);
		}
	}
}

static void Encoder_Task(void* parameter)
{
	int16_t new_count = 0;
	int16_t delta_count = 0;
	int16_t prev_count = 0;
	
	BaseType_t xReturn = pdTRUE;
	
	while(1)
	{
		new_count = RotEncoder_getCounter();
        delta_count = prev_count - new_count;
		prev_count = new_count;
		
		if (delta_count != 0)
		{
			//Notify FSM
			xReturn = xQueueSend( Encoder_Delta_Queue_Handle,
								&delta_count,
								0 );
			
			xEventGroupSetBits(FSM_Event_Handle, ENCODER_ROTATE_EVENT);
		}
		
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

static void FSM_Task(void* parameter)
{
	EventBits_t r_event;
	while(1)
	{
		r_event = xEventGroupWaitBits( FSM_Event_Handle,
									   BUTTON_PRESSED_EVENT|ENCODER_ROTATE_EVENT|SENSOR_UPDATE_EVENT|FOCUS_TIMEOUT_EVENT|BREAK_TIMEOUT_EVENT,
									   pdTRUE,
									   pdFALSE,
									   portMAX_DELAY );
		FSM_run(r_event);
			
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}


static void AppTaskCreate_Task(void* parameter)
{
	BaseType_t xReturn = pdPASS;
	
	taskENTER_CRITICAL();
	
	FocusTimer_SWTimer_Handle = xTimerCreateStatic(
                                "Focus Timer",
                                pdMS_TO_TICKS(FOCUS_TIME_MS),
                                pdFALSE,
                                NULL,
                                FocusTimer_SWTimer_Callback,
                                &Focus_Timer_TCB);
	if(FocusTimer_SWTimer_Handle != NULL)
		printf("Focus Timer succesfully created\r\n");
	
	BreakTimer_SWTimer_Handle = xTimerCreateStatic(
                                "Break Timer",
                                pdMS_TO_TICKS(BREAK_TIME_MS),
                                pdFALSE,
                                NULL,
                                BreakTimer_SWTimer_Callback,
                                &Break_Timer_TCB);
	if(BreakTimer_SWTimer_Handle != NULL)
		printf("Break Timer succesfully created\r\n");
	
	FSM_Event_Handle = xEventGroupCreate();
	if(FSM_Event_Handle != NULL)
		printf("FSM EventGroup succesfully created\r\n");
	
	UART_MuxSem_Handle = xSemaphoreCreateMutex();
	if(UART_MuxSem_Handle != NULL)
		printf("UART Semaphore succesfully created\r\n");
	xReturn = xSemaphoreGive(UART_MuxSem_Handle);
	
	Button_BiSem_Handle = xSemaphoreCreateBinary();
	if(Button_BiSem_Handle != NULL)
		printf("Button Semaphore succesfully created\r\n");
	
	Brightness_Queue_Handle = xQueueCreate((UBaseType_t)QUEUE_LEN,
										   sizeof(int16_t));
	if(Brightness_Queue_Handle != NULL)
		printf("Brightness Queue succesfully created\r\n");
	
	Encoder_Delta_Queue_Handle = xQueueCreate((UBaseType_t)QUEUE_LEN,
											sizeof(int16_t));
	if(Encoder_Delta_Queue_Handle != NULL)
		printf("Encoder Delta Queue succesfully created\r\n");
	
	Lux_Queue_Handle = xQueueCreate((UBaseType_t)QUEUE_LEN,
									sizeof(uint16_t));
	if(Lux_Queue_Handle != NULL)
		printf("Lux Queue succesfully created\r\n");
	
	
	LED_Task_Handle = xTaskCreateStatic( LED_Task,
										(const char*) "LED_Task",
										(u32)configMINIMAL_STACK_SIZE,
										(void*) NULL,
										(UBaseType_t)4,
										LED_Task_Stack,
										&LED_Task_TCB);
										
	Sensor_Task_Handle = xTaskCreateStatic( Sensor_Task,
										(const char*) "Sensor_Task",
										(u32)configMINIMAL_STACK_SIZE,
										(void*) NULL,
										(UBaseType_t)5,
										Sensor_Task_Stack,
										&Sensor_Task_TCB);
										
	Button_Task_Handle = xTaskCreateStatic( Button_Task,
										(const char*) "Button_Task",
										(u32)configMINIMAL_STACK_SIZE,
										(void*) NULL,
										(UBaseType_t)10,
										Button_Task_Stack,
										&Button_Task_TCB);
										
	Encoder_Task_Handle = xTaskCreateStatic( Encoder_Task,
										(const char*) "Encoder_Task",
										(u32)configMINIMAL_STACK_SIZE,
										(void*) NULL,
										(UBaseType_t)10,
										Encoder_Task_Stack,
										&Encoder_Task_TCB);
										
	FSM_Task_Handle = xTaskCreateStatic( FSM_Task,
										(const char*) "FSM_Task",
										(u32)configMINIMAL_STACK_SIZE,
										(void*) NULL,
										(UBaseType_t)9,
										FSM_Task_Stack,
										&FSM_Task_TCB);
										
										
	if (LED_Task_Handle != NULL && Sensor_Task_Handle != NULL && Button_Task_Handle != NULL && FSM_Task_Handle != NULL && Encoder_Task_Handle != NULL)
	printf("Task Created\r\n");
	else										
	printf("Task Not Created\r\n");
	
	vTaskDelete(AppTaskCreate_Handle);
	
	taskEXIT_CRITICAL();
}


/* -------------------- Delay Function -------------------- */

// Simple software delay using a busy loop
// NOTE:
// - Not accurate (depends on compiler optimization & clock)
// - CPU is fully occupied during delay (not efficient)
// - Only suitable for simple demos/tests
void Delay(__IO u32 nCount)
{
    for (; nCount != 0; nCount--);
}



void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
	*ppxIdleTaskTCBBuffer = &Idle_Task_TCB; //Stores the pointer to Idle_Task_TCB
	*ppxIdleTaskStackBuffer = Idle_Task_Stack; //Store pointer to first element of this array
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
									StackType_t **ppxIdleTaskStackBuffer,
									uint32_t *pulIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer = &Timer_Task_TCB;
	*ppxIdleTaskStackBuffer = Timer_Task_Stack;
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	
}

