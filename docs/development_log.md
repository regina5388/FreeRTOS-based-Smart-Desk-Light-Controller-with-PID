# Development Log

This document records the daily development progress, debugging notes, and design decisions of the **FreeRTOS-based Smart Focus Desk Light Controller** project.

The goal of this log is to document not only what was implemented, but also what problems were encountered, how they were debugged, and why certain design decisions were made.

---

## 2026-05-24 — Basic PWM Output Test

### Goal

Set up basic LED brightness control using STM32 timer PWM.

### Work Done

- Configured timer clock, GPIO alternate function output, and PWM output mode.
- Tested different duty cycles manually in `main()`:
  - `20%`
  - `60%`
  - `0%`
  - `80%`
  - `100%`
- Added a basic `Set_DutyCycle()` function for LED brightness control.

### Problems

- The LED brightness seemed almost unchanged during testing.
- Needed to confirm whether the PWM output and duty cycle update were actually working.

### Notes

PWM generation should be handled by hardware timers because LED dimming requires stable timing and should not be implemented using software delays.

### Next Step

- Add FreeRTOS task structure.
- Move LED brightness update into a dedicated task.

---

## 2026-05-24 — FreeRTOS Critical Sections and Task Blocking

### Goal

Understand how FreeRTOS protects shared resources and when tasks should block.

### Work Done

Studied the following FreeRTOS concepts:

- `taskENTER_CRITICAL()`
- `taskEXIT_CRITICAL()`
- ISR-safe critical sections
- Critical section nesting
- Blocking vs busy-waiting

### Design Decision

Tasks should block when they have nothing to do. This avoids unnecessary CPU usage and improves system responsiveness.

### Notes

For this project, tasks should wait on queues, semaphores, or event groups instead of continuously polling.

### Next Step

- Use queues and event groups for communication between tasks.

---

## 2026-05-30 — FreeRTOS Integration and Debugging

### Goal

Integrate FreeRTOS into the STM32 project and debug scheduler startup issues.

### Work Done

- Checked FreeRTOS configuration.
- Investigated SysTick and scheduler behavior.
- Debugged execution stopping in FreeRTOS startup code.
- Verified interrupt-related configuration.

### Problems

The system stopped unexpectedly during FreeRTOS execution.

Items checked:

- SysTick handler
- PendSV handler
- SVC handler
- Interrupt priority configuration
- `FreeRTOSConfig.h` settings

### Notes

FreeRTOS requires correct exception handlers and interrupt priority settings. Incorrect priority configuration can cause hard faults or scheduler failure.

### Next Step

- Create basic tasks and test task switching.

---

## 2026-05-31 — FSM and Event-Driven Task Design

### Goal

Design the system around a finite state machine.

### Work Done

Added basic FSM states:

```c
STATE_INIT
STATE_IDLE
STATE_AUTO_CONTROL
STATE_MANUAL_CONTROL
STATE_ERROR
```

Added an FSM task.

Discussed why `FSM_run()` should only wake up when events occur.

### Design Decision

The FSM should be event-driven instead of running continuously.

```text
No event -> FSM task blocks
Event occurs -> FSM task wakes up
FSM handles event
FSM task blocks again
```

This reduces CPU usage and makes the system behavior easier to reason about.

### Notes

This is better than calling `FSM_run()` continuously in a `while(1)` loop.

### Next Step

- Add EventGroup-based event handling.
- Add button and encoder events.

---

## 2026-05-31 — Manual Brightness Control with Encoder

### Goal

Implement manual brightness adjustment using a rotary encoder.

### Work Done

Added manual brightness variables:

```c
static int16_t manual_brightness = 0;
static int16_t current_brightness = 0;
static int16_t encoder_delta = 0;
```

Used encoder delta to update brightness.

Added brightness limiting between `0` and `100`.

### Problems

Needed to make sure queue item size was correct.

Earlier queue macros used:

```c
#define QUEUE_LEN   1
#define QUEUE_SIZE  1
```

This is unsafe if the queue stores `int16_t`.

### Fix

Use:

```c
xQueueCreate(1, sizeof(int16_t));
```

or the static queue equivalent.

### Design Decision

Use a queue length of `1` and overwrite the previous brightness value, because only the latest brightness command matters.

### Next Step

- Use `xQueueOverwrite()` for brightness updates.
- Add LED task to receive brightness commands.

---

## 2026-06-01 — Smart Desk Light Project Scope

### Goal

Define the project as a complete embedded system rather than a simple LED dimmer.

### Work Done

Defined the project direction:

```text
FreeRTOS-based Smart Focus Desk Light Controller
```

Planned features:

- Manual brightness control with rotary encoder
- Automatic brightness control using light sensor
- FSM-based mode control
- FreeRTOS tasks and queues
- Software timer based Pomodoro focus mode
- UART debug output
- Possible PID control
- Possible ToF gesture control

### Design Decision

The project should demonstrate embedded engineering skills:

- GPIO
- EXTI
- TIM PWM
- TIM Encoder
- I2C sensor
- UART debug
- FreeRTOS tasks
- Queue
- EventGroup
- Software Timer
- FSM
- Debugging
- Documentation

### Next Step

- Add FreeRTOS software timer for Pomodoro mode.

---

## 2026-06-02 — FreeRTOS Software Timer Study

### Goal

Understand whether a software timer is suitable for Pomodoro timing.

### Work Done

- Studied FreeRTOS software timers.
- Compared software timers with hardware timers.
- Decided that Pomodoro timing does not require hardware timer precision.

### Design Decision

Use FreeRTOS software timers for human-scale timing:

```text
Focus timer: 25 min
Break timer: 5 min
```

PWM and precise signal generation remain handled by hardware timers.

### Notes

Software timer callbacks should be short. They should not directly control hardware or perform long operations.

Recommended pattern:

```text
Software timer callback
        ↓
Set EventGroup bit
        ↓
FSM task handles state transition
```

### Next Step

- Add `STATE_FOCUS` and `STATE_BREAK`.

---

## 2026-06-03 — Pomodoro FSM States

### Goal

Add Pomodoro functionality to the FSM.

### Work Done

Added new states:

```c
STATE_FOCUS,
STATE_BREAK
```

Added events:

```c
#define FOCUS_TIMEOUT_EVENT  (0x01 << 4)
#define BREAK_TIMEOUT_EVENT  (0x01 << 5)
```

### Design Decision

Focus and Break should be separate FSM states instead of using one generic Pomodoro state.

Reason:

```text
FOCUS and BREAK have different timer durations,
different brightness behavior,
and different state transitions.
```

### Planned Behavior

```text
MANUAL / AUTO
    ↓ button
FOCUS
    ↓ focus timeout
BREAK
    ↓ break timeout
FOCUS
```

### Next Step

- Create Focus and Break software timers.

---

## 2026-06-04 — Static Software Timer Creation

### Goal

Create FreeRTOS software timers using static allocation.

### Work Done

Added static timer objects:

```c
static StaticTimer_t Focus_Timer_TCB;
static StaticTimer_t Break_Timer_TCB;

TimerHandle_t FocusTimer_SWTimer_Handle = NULL;
TimerHandle_t BreakTimer_SWTimer_Handle = NULL;
```

Created Focus timer:

```c
FocusTimer_SWTimer_Handle = xTimerCreateStatic(
                                "Focus",
                                pdMS_TO_TICKS(FOCUS_TIME_MS),
                                pdFALSE,
                                NULL,
                                FocusTimer_SWTimer_Callback,
                                &Focus_Timer_TCB);
```

### Design Decision

Use static allocation because it makes memory ownership explicit and avoids runtime heap allocation.

### Notes

`uxAutoReload = pdFALSE` was selected because Focus and Break timers are one-shot timers.

### Next Step

- Add callbacks that set FSM events.

---

## 2026-06-04 — Timer Callback Design

### Goal

Implement timer callbacks correctly.

### Work Done

Implemented callback structure:

```c
static void FocusTimer_SWTimer_Callback(TimerHandle_t xTimer)
{
    xEventGroupSetBits(FSM_Event_Handle, FOCUS_TIMEOUT_EVENT);
}

static void BreakTimer_SWTimer_Callback(TimerHandle_t xTimer)
{
    xEventGroupSetBits(FSM_Event_Handle, BREAK_TIMEOUT_EVENT);
}
```

### Design Decision

Timer callbacks should only post events to the FSM task.

They should not:

- Run long code
- Delay
- Control OLED/I2C directly
- Perform complex state logic

### Notes

The real state transition is handled inside `FSM_run()`.

### Next Step

- Debug whether the callbacks are entered at the expected time.

---

## 2026-06-05 — Timer Debug Variables

### Goal

Verify whether Focus and Break timers expire at the expected time.

### Work Done

Added debug tick variables:

```c
TickType_t focus_start_tick = 0;
TickType_t break_start_tick = 0;
TickType_t focus_timeout_tick = 0;
TickType_t break_timeout_tick = 0;
```

Used `xTaskGetTickCount()` to measure elapsed time.

### Problem

Linker reported multiple definition errors:

```text
Symbol focus_start_tick multiply defined
Symbol break_start_tick multiply defined
```

### Root Cause

Variables were defined inside a header file. The header was included by multiple `.c` files, causing each translation unit to define its own copy.

### Fix

In the header file:

```c
extern TickType_t focus_start_tick;
extern TickType_t break_start_tick;
extern TickType_t focus_timeout_tick;
extern TickType_t break_timeout_tick;
```

In exactly one `.c` file:

```c
TickType_t focus_start_tick = 0;
TickType_t break_start_tick = 0;
TickType_t focus_timeout_tick = 0;
TickType_t break_timeout_tick = 0;
```

### Lesson Learned

A global variable should be defined in one `.c` file only. Header files should only contain `extern` declarations.

### Next Step

- Use tick logs to check timer accuracy.

---

## 2026-06-05 — Timer Reset Bug

### Goal

Debug why the Focus timer callback was not entered.

### Problem

The timer callback was not triggered.

### Root Cause

`xTimerReset()` was placed directly inside the `STATE_FOCUS` case body.

This caused the timer to restart every time the FSM handled an event in Focus state.

Incorrect pattern:

```c
case STATE_FOCUS:
    xTimerReset(FocusTimer_SWTimer_Handle, 0);
    break;
```

### Fix

Move `xTimerReset()` to the state entry logic only.

Correct pattern:

```text
Entering FOCUS:
    record start tick
    reset/start Focus timer

Inside STATE_FOCUS:
    handle events only
```

### Design Decision

Timer operations should happen on state entry and state exit, not continuously inside the state body.

### Next Step

- Create helper functions such as `Enter_Focus_State()` and `Enter_Break_State()`.

---

## 2026-06-06 — Wrong Start Tick Debugging

### Goal

Verify timer elapsed time using UART output.

### Work Done

Printed Focus and Break elapsed time:

```text
Enter FOCUS, tick = 41304
FOCUS timeout, tick = 66304, elapsed = 25000 ms
Enter BREAK, tick = 66309
BREAK timeout, tick = 71309, elapsed = 5000 ms
```

### Problem

The first transition from Manual/Auto mode into Focus showed an incorrect elapsed time.

Example:

```text
Enter FOCUS, tick = 0
FOCUS timeout, tick = 36294, elapsed = 36294 ms
```

### Root Cause

When entering `STATE_FOCUS`, the code accidentally updated:

```c
break_start_tick = xTaskGetTickCount();
```

instead of:

```c
focus_start_tick = xTaskGetTickCount();
```

### Fix

Corrected the state entry code:

```c
focus_start_tick = xTaskGetTickCount();
xTimerReset(FocusTimer_SWTimer_Handle, 0);
```

### Lesson Learned

All state entry behavior should be placed in helper functions to avoid inconsistent behavior between different transition paths.

### Next Step

- Refactor state transitions into `Enter_Focus_State()` and `Enter_Break_State()`.

---

## 2026-06-06 — Verified Focus and Break Timer Accuracy

### Goal

Confirm that FreeRTOS software timers expire at the expected time.


### Result

The software timers expired at the expected time.

Measured values:

| Mode | Expected | Measured |
|---|---:|---:|
| Focus | 25 s | 25000 ms |
| Break | 5 s | 5000 ms |

### Notes

The small delay between timeout and next state entry is expected because the timer callback only posts an event to the FSM task.

### Next Step

- Replace test timing with real Pomodoro timing:
  - Focus: 25 min
  - Break: 5 min

---

## 2026-06-07 — README and Documentation Planning

### Goal

Plan how to document the project for GitHub.

### Implemented

- FreeRTOS task structure
- FSM states
- Manual control state
- Focus and Break states
- Static software timers
- Timer callbacks
- Timer debug logs
- UART debug output
- EventGroup-based timeout handling
- Used GPT generate my previous logs

### In Progress

- Refactoring state entry code into helper functions
- Cleaning up FSM transitions
- Adding LED behavior for Focus and Break states

### Next Tasks

- Implement `Enter_Focus_State()`
- Implement `Enter_Break_State()`
- Add brightness behavior for Focus and Break
- Add automatic light control using sensor input
- Add UART CLI commands
- Add unit tests for FSM transitions