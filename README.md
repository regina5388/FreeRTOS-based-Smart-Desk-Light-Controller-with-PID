# FreeRTOS-based Smart Desk Light Controller with PID

An STM32F103VE desk light controller using FreeRTOS, real I2C light-sensor feedback, PWM dimming, a rotary encoder, and an event-driven finite state machine.

The application implements automatic brightness control, manual adjustment, and timed focus/break modes. It is under development; the behavior and limitations below describe the current source code rather than a completed product.

## Implemented Features

- FreeRTOS V9.0.0 with preemptive scheduling and a 1 ms tick.
- Real lux measurements using a BH1750-compatible command set and `raw / 1.2f` conversion.
- Automatic brightness control through `PID_Update()`; current gains enable only the proportional term.
- LED PWM duty-cycle commands from 0 to 100%.
- Hardware timer encoder counting and manual brightness adjustment.
- Encoder push-button interrupts with task-based press/release debounce.
- Event-group coordination of button, encoder, sensor, and timer events.
- One-shot focus and break software timers, currently 10 seconds and 5 seconds for testing.
- Static application-task and software-timer storage, plus Idle/Timer task memory callbacks.
- UART diagnostics with mutex protection around most application-task output.

## Hardware and Build

The supplied Keil project targets **STM32F103VE (Cortex-M3)** with `STM32F10X_HD` and the STM32 Standard Peripheral Library. The encoder uses TIM8, so the current configuration requires a device with that peripheral.

| Function | Peripheral / pins | Configuration |
|---|---|---|
| External LED PWM | TIM2 channel 3 / PA2 | ARR = 999, prescaler = 71; 1 kHz with a 72 MHz timer clock |
| Encoder A/B | TIM8 channels 1/2 / PC6, PC7 | Pull-ups, encoder TI12 mode, 16-bit counter |
| Encoder button | PC10 / EXTI line 10 | Active low, pull-up, falling-edge interrupt |
| Light sensor | I2C1 / PB6 SCL, PB7 SDA | 400 kHz; default 7-bit address `0x23` (`0x46` in the driver) |
| UART diagnostics | USART1 / PA9 TX, PA10 RX | 115200 baud, 8 data bits, no parity, 1 stop bit |

Connect the sensor and encoder to the configured pins with a shared ground and suitable I2C pull-ups. PA2 provides the PWM control signal; use an appropriate LED driver for the lamp hardware.

1. Open [BH_F103.uvprojx](Desk_Light_Controller/Project/RVMDK%28uv5%29/BH_F103.uvprojx) in Keil uVision 5.
2. Use the configured `Target 1`. The project records ARM Compiler 5.06 update 7 and `Keil.STM32F1xx_DFP.2.3.0`.
3. Check the clock configuration and hardware pin mapping for your board, then build the target.
4. Configure the debug adapter and flash the board through uVision.
5. Open a serial terminal at **115200, 8N1** to inspect diagnostics.

No command-line build or automated hardware test workflow is supplied. The setup above is based on project configuration; it does not imply a verified build or hardware run.

## Task Architecture

`main()` calls `BSP_Init()`, creates `AppTaskCreate_Task` dynamically, and starts the scheduler. The creation task allocates RTOS objects, creates five static application tasks, and deletes itself.

| Task | Priority | Current behavior |
|---|---|---|
| `AppTaskCreate_Task` | 3 | Creates timers, event group, mutex, semaphore, queues, and application tasks |
| `LED_Task` | 4 | Polls the brightness queue without blocking, applies PWM, then delays 20 ms |
| `Sensor_Task` | 5 | Reads lux only in auto mode, with a 500 ms delay between reads; checks other modes every 100 ms |
| `Button_Task` | 10 | Waits for the ISR semaphore, debounces, waits for release, and posts a button event |
| `Encoder_Task` | 10 | Samples the encoder counter every 10 ms and queues a nonzero delta |
| `FSM_Task` | 9 | Waits for any relevant event bit, clears received bits, calls `FSM_run(r_event)`, then delays 20 ms |

The FSM is **event driven**, not executed at a fixed 10 ms period. Button debounce uses 20 ms after the press, 5 ms polling while held, and another 20 ms after release. A button action is posted after release.

The sensor is powered on and configured for continuous high-resolution measurement when auto mode becomes active. The task waits 180 ms before its first read and powers the sensor down when it observes another mode.

## RTOS Objects and Data Flow

All three queues have length one.

| Object | Payload / purpose |
|---|---|
| `Lux_Queue_Handle` | `float`; sensor overwrites the latest lux value for the FSM |
| `Encoder_Delta_Queue_Handle` | `int16_t`; encoder sends a counter delta for manual adjustment |
| `Brightness_Queue_Handle` | `int16_t`; FSM overwrites the latest PWM percentage for the LED task |
| `Button_BiSem_Handle` | Binary semaphore given by `EXTI15_10_IRQHandler()` |
| `UART_MuxSem_Handle` | Mutex used around application diagnostics |
| `FSM_Event_Handle` | Button, encoder, sensor-update, focus-timeout, and break-timeout bits |
| `FocusTimer_SWTimer_Handle` | Static one-shot timer posting `FOCUS_TIMEOUT_EVENT` |
| `BreakTimer_SWTimer_Handle` | Static one-shot timer posting `BREAK_TIMEOUT_EVENT` |

```text
Light sensor -> Sensor_Task -> Lux queue + sensor event -> FSM / PID
Encoder      -> Encoder_Task -> Delta queue + encoder event -> FSM
Button EXTI  -> Binary semaphore -> Button_Task -> button event -> FSM
Timers       -> timeout events -> FSM
FSM          -> Brightness queue -> LED_Task -> TIM2 PWM
```

Application tasks and both software timers use static storage. The creation task, queues, semaphore, mutex, and event group use dynamic allocation backed by `heap_4.c`. Both allocation modes are enabled; the configured FreeRTOS heap is 36 KiB.

## Operating Modes

`BSP_Init()` calls `FSM_init()`, setting `STATE_INIT`. The first event processed by the FSM initializes the PID and changes the state to `STATE_IDLE`; that event is not processed again in the idle state.

| Current state | Event | Result |
|---|---|---|
| `STATE_INIT` | First event | Initialize PID and enter idle |
| `STATE_IDLE` | Button | Enter auto control, reset PID, and set the lux target |
| `STATE_AUTO_CONTROL` | Sensor update | Read lux, calculate brightness, and publish PWM command |
| `STATE_AUTO_CONTROL` | Button | Enter manual control, retaining the commanded brightness |
| `STATE_MANUAL_CONTROL` | Encoder rotation | Adjust brightness and clamp it to 0-100% |
| `STATE_MANUAL_CONTROL` | Button | Enter focus mode and start the focus timer |
| `STATE_FOCUS` | Focus timeout | Enter break mode and start the break timer |
| `STATE_FOCUS` | Button | Stop focus timer and return to auto |
| `STATE_BREAK` | Break timeout | Restart focus mode and its timer |
| `STATE_BREAK` | Button | Stop break timer and return to auto |

Normal button navigation after initialization is:

```text
IDLE -> AUTO -> MANUAL -> FOCUS -> AUTO
                           |
                    10-second timeout
                           v
                         BREAK
                           |
                     5-second timeout
                           v
                         FOCUS
```

Manual brightness changes by `encoder_delta * STEP`, with `STEP = 5` percentage points per timer count. The delta is calculated as `previous_count - new_count`; the change per physical detent depends on the encoder.

Focus mode retains the commanded brightness and does not read the sensor. On each invocation of the break-state handler, the FSM commands 0%, waits one second, then restores the saved brightness. This is event-triggered behavior, not continuous periodic blinking.

`STATE_SETF_TEST` is an empty placeholder. `STATE_ERROR` only prints an error message; no automatic fault transition or recovery is implemented. `INTIALIZE_DONE_EVENT` is declared but is not posted or included in the FSM wait mask.

## Automatic Brightness Control

The current settings in [state_fsm.c](Desk_Light_Controller/User/Src/state_fsm.c) are:

| Parameter | Value |
|---|---|
| Lux target | 200 lx |
| `Kp`, `Ki`, `Kd` | `0.5`, `0.0`, `0.0` |
| PID time step | 0.5 seconds |
| Output limits | 0-100% |
| Change limit per update | -30 to +30 percentage points |
| Integral limits | -1000 to +1000 |

`PID_Update()` computes proportional, integral, and derivative terms, clamps their sum to the change limits, adds that change to the previous output, and clamps the resulting PWM command. The float output is converted to `int16_t` before being queued. With the current gains, only the proportional term contributes to each output change.

The 0.5-second PID time step corresponds to the sensor task's nominal delay; actual updates also include I2C, logging, and scheduling time. The `LUX_MIN` and `LUX_MAX` constants are currently unused, and the encoder does not adjust the lux target.

## Diagnostics

Startup output includes `TEST starts now`, timer/event-group/semaphore/queue creation messages, and `Task Created` when all five application task handles are valid.

Runtime messages include:

- `Sent lux: ... lx` and `Auto ! lux measured is ...` in auto mode.
- `Receive Brightness ! ...` when the LED task receives a command.
- `delta=...` for encoder events handled in manual mode.
- Focus/break entry and timeout messages containing ticks and elapsed milliseconds.

UART output is polling based. Most task messages take the UART mutex, but I2C debug macros and `FSM_setState()` print without it, so output protection is not universal.

## Current Limitations

- **Startup depends on an external event.** No initialization event is generated. The first button release or encoder movement only advances `STATE_INIT` to idle; another button action is needed to enter auto mode.
- **Initial LED command is undefined.** PWM hardware starts at 50%, but `LED_Task` uses an uninitialized `Brightness_buf` before the first successful queue receive and applies it even when reception fails. Startup brightness is therefore not reliable.
- **Break indication is delayed.** Entering break mode does not immediately execute its handler. If no button or encoder event arrives, the off/restore operation happens when the break-timeout event is handled, followed by the return to focus. The one-second delay also blocks FSM event handling.
- **Encoder updates can be lost.** A nonblocking send to the single-slot delta queue can fail when full, but the encoder task still posts an event. The queue is only consumed in manual mode, so a delta from another mode can remain pending.
- **PID reset uses lux as an output value.** Calls to `PID_Reset(&pid, current_lux)` seed a PWM output from a lux measurement and clamp it to 0-100, rather than preserving the current PWM command.
- **Fault handling is incomplete.** Failed sensor reads do not post an update or enter `STATE_ERROR`; most RTOS creation and timer-command failures have no recovery path.
- **Timer callbacks may block on UART.** Both callbacks take the UART mutex with `portMAX_DELAY`, which can delay the shared software-timer service task.
- **Timer debug is effectively required.** Tick variables are guarded by `TIMER_DEBUG`, but uses in the callbacks and FSM are unconditional. Simply setting it to zero requires code changes.
- **Timer task stack reporting differs from configuration.** Its buffer is allocated with `configTIMER_TASK_STACK_DEPTH` (256 words), but the memory callback reports `configMINIMAL_STACK_SIZE` (128 words).

## Future Work

- Correct startup, queue handling, and mode-entry behavior.
- Tune and validate brightness control on the lamp hardware.
- Add nonblocking break reminders and robust fault handling.
- Add UART commands and runtime parameter tuning.
- Integrate CAN communication and an optional Linux/SocketCAN controller.
- Add data logging, monitoring, and additional lighting modes.

OLED, SPI flash, ADC, FatFS, and other support sources are present in the project, but the current desk-light application does not integrate them into its task flow. CAN peripheral library code is included; application-level CAN communication is not implemented.

## Source Layout

- [main.c](Desk_Light_Controller/User/main.c): tasks, RTOS objects, timer callbacks, and static memory callbacks.
- [state_fsm.c](Desk_Light_Controller/User/Src/state_fsm.c): mode transitions and brightness commands.
- [pid.c](Desk_Light_Controller/User/Src/pid.c): PID calculations, limits, reset, and tuning functions.
- [bsp_i2c_light_sensor.c](Desk_Light_Controller/User/Src/bsp_i2c_light_sensor.c): sensor commands and lux conversion.
- [bsp_rot_encoder.c](Desk_Light_Controller/User/Src/bsp_rot_encoder.c): encoder/button hardware setup.
- [bsp_timer.c](Desk_Light_Controller/User/Src/bsp_timer.c): LED PWM setup and duty-cycle updates.
- [stm32f10x_it.c](Desk_Light_Controller/User/stm32f10x_it.c): interrupt handlers, including the button ISR.
- [FreeRTOSConfig.h](Desk_Light_Controller/User/FreeRTOSConfig.h): scheduler, allocation, timer, and interrupt settings.
