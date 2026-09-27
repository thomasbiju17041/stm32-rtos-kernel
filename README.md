# Mini RTOS for STM32F407

A learning-focused project to build a preemptive RTOS from scratch on the STM32F407 Discovery board.

The goal of this project is to understand how a Cortex-M RTOS works internally, from task creation and stack initialization to exception handling, scheduling, context switching, delays, and synchronization, without relying on an existing RTOS library.

---

## Current Features

- Preemptive scheduling using PendSV
- SysTick-driven scheduling
- Priority-based task selection
- Round-robin scheduling for equal-priority tasks
- Time slicing
- Task states (`READY`, `RUNNING`, `BLOCKED`)
- Task delays using `task_delay()`
- Dedicated per-task stacks
- Task Control Blocks (TCBs)
- PSP-based thread execution
- MSP-based exception/handler execution
- SVC-based scheduler startup
- Manual Cortex-M exception stack frame initialization
- PendSV-based context switching
- Hardware context save/restore (`R4-R11`)
- Task blocking and wake-up
- SysTick-driven delayed-task wake-up
- Idle task
- Ready queue management
- Binary semaphores
- Counting semaphores

---

## Hardware

- **STM32F407 Discovery**

---

## IDE

- **STM32CubeIDE**
- **STM32CubeMX**
- ARM GCC toolchain

---

## Progress

### Completed

- [x] Task Control Blocks (TCBs)
- [x] Dedicated task stacks
- [x] Cortex-M startup stack frame creation
- [x] PSP initialization
- [x] Thread Mode execution using PSP
- [x] SVC-based scheduler startup
- [x] PendSV context switching
- [x] Hardware context save/restore (`R4-R11`)
- [x] Priority-based task selection
- [x] Task states (`READY`, `RUNNING`, `BLOCKED`)
- [x] Task blocking
- [x] SysTick wake-up mechanism
- [x] `task_delay()`
- [x] Preemptive task switching
- [x] Idle task
- [x] Round-robin scheduling
- [x] Time slicing
- [x] Ready queue management
- [x] Scheduler improvements
- [x] Binary semaphores
- [x] Counting semaphores

---

## Kernel Architecture

The kernel follows the typical Cortex-M RTOS architecture:

```text
                    ┌──────────────────┐
                    │    Application   │
                    │      Tasks       │
                    └────────┬─────────┘
                             │
                             ▼
                    ┌──────────────────┐
                    │    Scheduler     │
                    │ Priority Based   │
                    │ + Round Robin    │
                    └────────┬─────────┘
                             │
                ┌────────────┼────────────┐
                │            │            │
                ▼            ▼            ▼
            SysTick        PendSV         SVC
          Time Base    Context Switch   Startup
                │            │
                └─────┬──────┘
                      ▼
             ┌──────────────────┐
             │   Cortex-M CPU   │
             │       PSP        │
             └──────────────────┘
