# Mini RTOS for STM32F407

Learning project to build a preemptive RTOS from scratch on the STM32F407 Discovery board, focusing on understanding how Cortex-M kernels work internally rather than relying on existing RTOS libraries.

---

## Current Features

- Preemptive scheduling using PendSV
- SysTick-driven task scheduling
- Priority-based task selection
- Task states (READY, RUNNING, BLOCKED)
- Task delays (`task_delay()`)
- Dedicated per-task stacks
- Task Control Blocks (TCBs)
- PSP-based thread execution
- Manual Cortex-M exception stack initialization
- Full context switching (R4-R11 save/restore)

---

## Hardware

STM32F407 Discovery

---

## IDE

STM32CubeIDE

---

## Progress

### Completed

- [x] Task Control Blocks (TCBs)
- [x] Dedicated task stacks
- [x] Cortex-M startup stack frame creation
- [x] PSP initialization
- [x] Thread mode execution using PSP
- [x] SVC-based scheduler startup
- [x] PendSV context switching
- [x] Hardware context save/restore (R4-R11)
- [x] Priority scheduler
- [x] Task blocking
- [x] SysTick wake-up mechanism
- [x] `task_delay()`

### In Progress

- [ ] Idle task
- [ ] Round-robin scheduling for equal priorities
- [ ] Time slicing
- [ ] Ready queue improvements

### Planned

- [ ] Binary semaphores
- [ ] Counting semaphores
- [ ] Mutexes
- [ ] Priority inheritance


---

## Key Concepts Explored

- Cortex-M exception model
- MSP vs PSP
- SVC handler
- PendSV handler
- SysTick scheduling
- Context switching
- Exception stack frames
- Task Control Blocks (TCBs)
- Priority scheduling
- Blocking scheduler design

---

## Current Status

The kernel now supports preemptive context switching using PendSV, SysTick-driven scheduling, priority-based task selection, 
and blocking delays through `task_delay()`. The next development phase focuses on improving the scheduler with an idle task, 
round-robin scheduling, and synchronization primitives such as semaphores and mutexes.