# Mini RTOS for STM32F407

Learning project to build an RTOS from scratch on STM32F407 Discovery.

## Current Features

* Cooperative scheduler
* Task states (READY, RUNNING, BLOCKED)
* Task priorities
* Task delays
* Per-task stacks
* Task Control Blocks (TCBs)
* PSP/MSP exploration
* Cortex-M startup stack frame initialization
* Context switch framework
* Software context save/restore simulation

## Hardware

STM32F407 Discovery

## IDE

STM32CubeIDE

## Progress

### Completed

* [x] Cooperative scheduler
* [x] Task states
* [x] Task delays
* [x] Priority scheduling
* [x] Per-task stacks
* [x] Task Control Blocks (TCBs)
* [x] Fake Cortex-M stack frames
* [x] PSP initialization
* [x] Thread mode execution using PSP
* [x] Current task tracking
* [x] Context switch framework
* [x] Software simulation of context save/restore

### In Progress

* [ ] Real CPU register context save (R4-R11)
* [ ] Real CPU register context restore (R4-R11)
* [ ] PendSV-based context switching
* [ ] Task resume from saved execution state

### Planned

* [ ] Preemptive scheduling
* [ ] Semaphores
* [ ] Mutexes
* [ ] Message queues
* [ ] Inter-task communication

## Key Concepts Explored

* Cortex-M stack architecture
* MSP vs PSP
* Task Control Blocks (TCBs)
* Task stack initialization
* Context switching fundamentals
* Priority-based scheduling
* Exception stack frames
* Process Stack Pointer (PSP)

## Current Status

The kernel currently supports task scheduling, priorities, delays, dedicated task stacks, PSP initialization, and a software model of context save/restore. Work is now focused on implementing real Cortex-M context switching using PendSV and task stack restoration.
