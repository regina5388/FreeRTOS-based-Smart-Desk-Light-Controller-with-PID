## 2026-05-24–2026-06-07 — Realized Basic Functions

### Goal

Build the basic structure of the **FreeRTOS-based Smart Focus Desk Light Controller** and verify the core functions step by step.

The main goal during this period was to move the project from simple peripheral testing to a more structured embedded system based on FreeRTOS tasks, event groups, queues, software timers, and a finite state machine.

---

### Work Done


### Problems Met

#### PWM Brightness Test

At the beginning, different PWM duty cycles were tested manually, but the LED brightness seemed almost unchanged.

This required checking whether the PWM output was configured correctly and whether the duty cycle update function was actually working.

Problem was AFIO clock not enabled

---

#### FreeRTOS Integration Issues

During FreeRTOS integration, the system stopped unexpectedly during execution.

The following areas needed to be checked:

- SysTick handler
- PendSV handler
- SVC handler
- Interrupt priority configuration
- `FreeRTOSConfig.h`
- Scheduler startup behavior

This showed that FreeRTOS integration depends strongly on correct interrupt and exception configuration.

---

#### Queue Item Size Problem

The brightness queue was initially configured with macros such as:

```c
#define QUEUE_LEN   1
#define QUEUE_SIZE  1
```

This is unsafe if the queue stores an `int16_t`.

The correct queue item size should be:

```c
sizeof(int16_t)
```

because the queue copies data by byte size.

---



### Design Decisions

#### Event-Driven FSM

The FSM is designed to run only when events occur.

```text
No event -> FSM task blocks
Event occurs -> FSM task wakes up
FSM handles event
FSM task blocks again
```

This reduces CPU usage and makes the system easier to reason about.

---

#### Queue Length of 1 for Brightness

The brightness queue should have length `1`.

Only the latest brightness command matters, so old brightness values should not accumulate.

The intended design is:

```c
xQueueOverwrite(Brightness_Queue_Handle, &current_brightness);
```

---

#### Software Timer for Pomodoro Mode

The Pomodoro timer is implemented using FreeRTOS software timers because it is a human-time-scale function and does not require hardware timer precision.

PWM generation remains handled by hardware timers.

---

#### Timer Callback Only Posts Events

Timer callbacks should stay short.

The callback only sets an EventGroup bit:

```c
xEventGroupSetBits(FSM_Event_Handle, FOCUS_TIMEOUT_EVENT);
```

The actual state transition is handled inside the FSM task.

---

#### Focus and Break as Separate States

`STATE_FOCUS` and `STATE_BREAK` are separate states because they have different behavior:

- Different timer durations
- Different brightness behavior
- Different state transitions
- Different user interaction logic

---

### Current Status

#### Implemented

- Basic PWM brightness control
- FreeRTOS task structure
- Event-driven FSM structure
- Manual brightness control state
- Focus and Break states
- Static FreeRTOS software timers
- Timer callbacks
- EventGroup-based timeout handling
- UART debug output
- Timer elapsed-time verification

---

#### Verified

Shortened software timer test:

| Mode | Expected | Measured |
|---|---:|---:|
| Focus | 25 s | 25000 ms |
| Break | 5 s | 5000 ms |

This confirms that the software timers work correctly in the current test configuration.

---

### Next Steps

- Refactor state-entry logic into helper functions, such as:
  - `Enter_Focus_State()`
  - `Enter_Break_State()`
  - `Enter_Auto_State()`
  - `Enter_Manual_State()`
- Add brightness behavior for Focus and Break modes.
- Add automatic brightness control using a light sensor.
- Add PID logic for smoother automatic brightness adjustment.
- Add UART CLI commands for runtime configuration.
