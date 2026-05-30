#include "bsp_initialize.h"

static StackType_t Idle_Task_Stack[configMINIMAL_STACK_SIZE];
static StackType_t Timer_Task_Stack[configTIMER_TASK_STACK_DEPTH];
static StackType_t LED_Task_Stack[configTIMER_TASK_STACK_DEPTH];

static StaticTask_t Idle_Task_TCB;
static StaticTask_t Timer_Task_TCB;
static StaticTask_t LED_Task_TCB;

static TaskHandle_t AppTaskCreate_Handle;
static TaskHandle_t LED_Task_Handle;


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
	while(1)
	{
		Set_DutyCycle_LED(0);
		vTaskDelay(500); //Delay 500 Tick
		
		
		Set_DutyCycle_LED(100);
		vTaskDelay(500); //Delay 500 Tick
	}
	
}
static void AppTaskCreate_Task(void* parameter)
{
	taskENTER_CRITICAL();
	LED_Task_Handle = xTaskCreateStatic( LED_Task,
										(const char*) "LED_Task",
										(u32)configMINIMAL_STACK_SIZE,
										(void*) NULL,
										(UBaseType_t)4,
										LED_Task_Stack,
										&LED_Task_TCB);
	if (LED_Task_Handle != NULL)
	printf("Task Created");
	else										
	printf("Task Not Created");
	
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

