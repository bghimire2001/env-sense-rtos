# env-sense-rtos
ESP32-based real-time environmental monitoring system using FreeRTOS, featuring multi-task scheduling, sensor data acquisition (light + humidity), and inter-task communication via queues and interrupts.

## Overview 
Core Embedded Systems concepts include:
- Real-time task scheduling with FreeRTOS
- Sensor data acquisition (light + humidity)
- Inter-task communication using queues
- Interrupt-driven event handling
- Hardware/Software integration on ESP32

Designed to simulate a scalable embedded monitoring node, where multiple inputs are processed concurrently and reliably.

                +----------------------+
                |     Humidity Task    |
                +----------+-----------+
                           |
                           v
                     +-----------+
                     |   Queue   |
                     +-----------+
                           ^
                           |
                +----------+-----------+
                |     Light Task       |
                +----------+-----------+
                           |
                           v
                +----------------------+
                |   Processing Task    |
                | (filter / threshold) |
                +----------+-----------+
                           |
                           v
                +----------------------+
                | Output / Logging     |
                | (UART / Serial)      |
                +----------------------+

Interrupt (Button / Event) ---> ISR ---> Queue/Event Trigger

## Features
- Multitasking with FreeRTOS
-   Seperate tasks for each sensor and processing logic
- Sensor Integration
-   Light sensor (analog inputs)
-   Humidity sensor (digital input)
- Inter-task communication
-   Queue-based data passing between tasks
- Interrupt Handling
-   Triggered by external input (e.g., button)
-   Sends signal to processing task
- Output
-   Logs data and system state via UART
