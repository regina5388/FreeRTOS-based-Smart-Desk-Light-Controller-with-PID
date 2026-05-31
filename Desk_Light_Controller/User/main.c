#include "bsp_initialize.h"

#define QUEUE_LEN 		4
#define QUEUE_SIZE 		4

static StackType_t Idle_Task_Stack[configMINIMAL_STACK_SIZE];
static StackType_t Timer_Task_Stack[configTIMER_TASK_STACK_DEPTH];
static StackType_t LED_Task_Stack[configTIMER_TASK_STACK_DEPTH];
static StackType_t Sensor_Task_Stack[configTIMER_TASK_STACK_DEPTH];


static StaticTask_t Idle_Task_TCB;
static StaticTask_t Timer_Task_TCB;
static StaticTask_t LED_Task_TCB;
static StaticTask_t Sensor_Task_TCB;


static TaskHandle_t AppTaskCreate_Handle;
static TaskHandle_t LED_Task_Handle;
static TaskHandle_t Sensor_Task_Handle;

static QueueHandle_t Brightness_Queue_Handle;



void Delay(__IO uint32_t nCount);
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize);

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
									StackType_t **ppxIdleTaskStackBuffer,
									uint32_t *pulIdleTaskStackSize);

static void LED_Task(void* parameter);
static void AppTaskCreate_Task(void* parameter);
	
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
	{	printf("AppTaskCreated\r\n");
		vTaskStartScheduler();
		printf("Schedular not started");
	}

    for (;;);
}


static void LED_Task(void* parameter)
{
	BaseType_t xReturn = pdTRUE;
	uint32_t Brightness_buf;
	while(1)
	{
		xReturn = xQueueReceive( Brightness_Queue_Handle,
							  &Brightness_buf,
							  1000 );
		if (xReturn == pdTRUE)
			printf("message received\r\n");
		else
			printf("receive error, error code 0x%1x\n", (uint32_t)xReturn);
		
		Set_DutyCycle_LED(Brightness_buf);
	}
	
}

static void Sensor_Task(void* parameter)
{
	BaseType_t xReturn = pdTRUE;
	uint32_t Brightness_1 = 0;
	uint32_t Brightness_2 = 20;
	uint32_t Brightness_3 = 100;
	
	while(1)
	{
		printf("Send Brightness\r\n");
		xReturn = xQueueSend( Brightness_Queue_Handle,
							  &Brightness_1,
							  0 );
		if (xReturn == pdTRUE)
			printf("lights off message sent\r\n");
		vTaskDelay(500); //Delay 500 Tick
		
				printf("Send Brightness");
		xReturn = xQueueSend( Brightness_Queue_Handle,
							  &Brightness_2,
							  0 );
		if (xReturn == pdTRUE)
			printf("lights dim message sent\r\n");
		vTaskDelay(500); //Delay 500 Tick
		
				printf("Send Brightness");
		xReturn = xQueueSend( Brightness_Queue_Handle,
							  &Brightness_3,
							  0 );
		if (xReturn == pdTRUE)
			printf("lights on message sent\r\n");
		vTaskDelay(500); //Delay 500 Tick

		
		
	}
	
}

static void AppTaskCreate_Task(void* parameter)
{
	taskENTER_CRITICAL();
	
	Brightness_Queue_Handle = xQueueCreate((UBaseType_t)QUEUE_LEN,
										   (UBaseType_t)QUEUE_SIZE);
	if(Brightness_Queue_Handle != NULL)
		printf("Brightness Queue succesfully created\r\n");
	
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
	if (LED_Task_Handle != NULL && Sensor_Task_Handle != NULL)
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

