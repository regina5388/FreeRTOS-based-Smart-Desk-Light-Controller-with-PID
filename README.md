# FreeRTOS-based Smart Desk Light Controller with PID

This project is a FreeRTOS-based smart desk light controller project based on STM32.

The current version implements the basic FreeRTOS software architecture, including task creation, static task memory allocation, queue-based LED brightness control, button interrupt handling with a binary semaphore, UART output protection with a mutex, and a finite state machine task.

The project is still under development. The current focus is to build a clean RTOS-based software structure before adding real sensor feedback, PID control, CAN communication, and more advanced smart light functions.

---

## Development Log

A daily development log is available here:

[Development Log](docs/development_log.md)

## Current Features

- FreeRTOS task-based software structure
- Static memory allocation for FreeRTOS Idle Task and Timer Task
- LED brightness control through a FreeRTOS queue
- Simulated sensor task for sending test brightness values
- Button event handling using binary semaphore
- Software debounce for encoder button
- UART `printf` protection using mutex
- Periodic finite state machine execution
- Basic mode switching between auto mode and manual mode

---

## FreeRTOS Objects Used

| Object | Name | Usage |
|---|---|---|
| Queue | `Brightness_Queue_Handle` | Transfers brightness values from `Sensor_Task` to `LED_Task` |
| Binary Semaphore | `Button_BiSem_Handle` | Wakes up `Button_Task` after a button interrupt |
| Mutex | `UART_MuxSem_Handle` | Protects UART `printf` output |
| Static Task Memory | `StaticTask_t` and `StackType_t` | Provides memory for statically created tasks |
| Task Delay | `vTaskDelay()` | Used for periodic task execution and button debounce |

---

## Task Overview

### 1. AppTaskCreate_Task

`AppTaskCreate_Task` is responsible for system-level FreeRTOS initialization.

It creates:

- UART mutex
- Button binary semaphore
- Brightness queue
- LED task
- Sensor task
- Button task
- FSM task

Main RTOS objects:

```c
UART_MuxSem_Handle = xSemaphoreCreateMutex();

Button_BiSem_Handle = xSemaphoreCreateBinary();

Brightness_Queue_Handle = xQueueCreate((UBaseType_t)QUEUE_LEN,
                                       (UBaseType_t)QUEUE_SIZE);
```

Application tasks are created using static task creation:

```c
LED_Task_Handle = xTaskCreateStatic(LED_Task,
                                    "LED_Task",
                                    configMINIMAL_STACK_SIZE,
                                    NULL,
                                    4,
                                    LED_Task_Stack,
                                    &LED_Task_TCB);
```

After initialization, this task deletes itself:

```c
vTaskDelete(AppTaskCreate_Handle);
```

---

### 2. LED_Task

`LED_Task` waits for brightness values from the brightness queue.

```c
xQueueReceive(Brightness_Queue_Handle,
              &Brightness_buf,
              1000);
```

After receiving a value, it updates the LED PWM duty cycle:

```c
Set_DutyCycle_LED(Brightness_buf);
```

Current brightness values are simulated by `Sensor_Task`.

---

### 3. Sensor_Task

`Sensor_Task` currently simulates brightness values for testing.

It sends three brightness values repeatedly:

```c
uint32_t Brightness_1 = 0;
uint32_t Brightness_2 = 20;
uint32_t Brightness_3 = 100;
```

The values are sent to `Brightness_Queue_Handle`:

```c
xQueueSend(Brightness_Queue_Handle, &Brightness_1, 0);
vTaskDelay(500);

xQueueSend(Brightness_Queue_Handle, &Brightness_2, 0);
vTaskDelay(500);

xQueueSend(Brightness_Queue_Handle, &Brightness_3, 0);
vTaskDelay(500);
```

Current test sequence:

```text
0% -> 20% -> 100%
```

This is used to verify that queue communication and PWM brightness control work correctly.

---

### 4. Button_Task

`Button_Task` handles button press events.

The task waits for a binary semaphore:

```c
xSemaphoreTake(Button_BiSem_Handle, portMAX_DELAY);
```

The semaphore is expected to be given by the button EXTI interrupt service routine.

After receiving the semaphore, the task performs software debounce:

```c
vTaskDelay(pdMS_TO_TICKS(20));
```

Then it checks whether the button is still pressed:

```c
if (GPIO_ReadInputDataBit(ENCODER_KEY_GPIO_PORT, ENCODER_KEY_GPIO_PIN) == BUTTON_ON)
```

If the press is valid, the task waits until the button is released:

```c
while (GPIO_ReadInputDataBit(ENCODER_KEY_GPIO_PORT, ENCODER_KEY_GPIO_PIN) == BUTTON_ON)
{
    vTaskDelay(pdMS_TO_TICKS(5));
}
```

After release, another debounce delay is applied:

```c
vTaskDelay(pdMS_TO_TICKS(20));
```

Current button function:

```text
STATE_IDLE / STATE_AUTO_CONTROL -> STATE_MANUAL_CONTROL
STATE_MANUAL_CONTROL            -> STATE_AUTO_CONTROL
STATE_ERROR                     -> no state switching
```

The current state is checked by:

```c
FSM_getState();
```

The state is changed by:

```c
FSM_setState(STATE_MANUAL_CONTROL);
FSM_setState(STATE_AUTO_CONTROL);
```

---

### 5. FSM_Task

`FSM_Task` runs the system finite state machine periodically.

```c
static void FSM_Task(void* parameter)
{
    while(1)
    {
        FSM_run();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

The FSM is executed every 10 ms.

Current system states include:

```text
STATE_IDLE
STATE_AUTO_CONTROL
STATE_MANUAL_CONTROL
STATE_ERROR
```

The FSM is used to manage the operating mode of the smart desk light.

---

## UART Mutex Protection

Because multiple tasks may use `printf`, a mutex is used to protect UART output.

The mutex is created during initialization:

```c
UART_MuxSem_Handle = xSemaphoreCreateMutex();
```

Before printing, a task takes the mutex:

```c
xSemaphoreTake(UART_MuxSem_Handle, portMAX_DELAY);
printf("button pressed\r\n");
xSemaphoreGive(UART_MuxSem_Handle);
```

This prevents output from different tasks from being mixed together.

---

## Static Memory Allocation

The project provides static memory for the FreeRTOS Idle Task and Timer Task.

Idle task memory:

```c
static StackType_t Idle_Task_Stack[configMINIMAL_STACK_SIZE];
static StaticTask_t Idle_Task_TCB;
```

Timer task memory:

```c
static StackType_t Timer_Task_Stack[configTIMER_TASK_STACK_DEPTH];
static StaticTask_t Timer_Task_TCB;
```

The required FreeRTOS callback functions are implemented:

```c
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &Idle_Task_TCB;
    *ppxIdleTaskStackBuffer = Idle_Task_Stack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
```

```c
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &Timer_Task_TCB;
    *ppxIdleTaskStackBuffer = Timer_Task_Stack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
```

---

## Current Data Flow

```text
+----------------------+
|     Sensor_Task       |
| Simulated brightness  |
+----------+-----------+
           |
           | FreeRTOS Queue
           v
+----------------------+
|      LED_Task         |
| Set PWM duty cycle    |
+----------------------+
```

Button event flow:

```text
+----------------------+
|      EXTI ISR         |
| Button interrupt      |
+----------+-----------+
           |
           | Binary Semaphore
           v
+----------------------+
|     Button_Task       |
| Debounce + FSM switch |
+----------------------+
```

FSM execution flow:

```text
+----------------------+
|      FSM_Task         |
| FSM_run() every 10 ms |
+----------------------+
```

UART protection:

```text
+----------------------+
|    UART Mutex         |
| Protect printf output |
+----------------------+
```

---

## Current Test Behavior

After startup, the program prints:

```text
TEST starts now
UART Semaphore succesfully created
Button Semaphore succesfully created
Brightness Queue succesfully created
Task Created
```

The sensor task periodically prints:

```text
Send Brightness
```

The LED brightness changes between:

```text
0%
20%
100%
```

When the encoder button is pressed, the program prints:

```text
button pressed
state x
```

Then the system state switches between manual mode and auto mode.

---

## Current Limitations

The current version uses simulated brightness values instead of a real light sensor.

PID control has not been fully integrated yet.

CAN communication has not been added yet.

The rotary encoder rotation function has not been added yet.

The current LED task directly applies the received brightness value to PWM duty cycle.

---

## Planned Features

Future features:

- Real light sensor input
- PID brightness control
- Rotary encoder brightness adjustment
- UART command parser
- Runtime parameter tuning
- CAN communication
- Error handling state
- Auto mode
- Manual mode
- Night mode
- Focus mode
- Linux main controller through CAN
- Data logging and system monitoring

---

## Possible Final System Design

The final smart desk light node may include:

```text
STM32 Smart Light Node
    - FreeRTOS
    - FSM
    - PID controller
    - PWM LED dimming
    - Light sensor
    - Rotary encoder
    - Button input
    - UART debug interface
    - CAN communication
```

Optional distributed system:

```text
Linux Main Controller
    |
    | CAN Bus
    |
STM32 Smart Light Node
```

The Linux controller can be used for:

- SocketCAN communication
- Data logging
- Web interface
- PID parameter tuning
- System status monitoring

---

## Notes

This project is under development.

The current version focuses on building a clean FreeRTOS-based application structure.

The next step is to replace the simulated sensor values with real sensor input and integrate PID-based brightness control.