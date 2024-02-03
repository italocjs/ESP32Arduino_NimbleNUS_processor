# BLE Communication with FreeRTOS on ESP32

This project demonstrates how to use the ESP32's Bluetooth Low Energy (BLE) capabilities in conjunction with FreeRTOS tasks and queues to achieve smooth operation under high load conditions. It also incorporates packetization for efficient data handling. The example is built using the Angel library for BLE communication and is designed to serve as a reference for both the project creator and other users interested in similar applications.

## Overview

The ESP32 microcontroller is known for its powerful features including WiFi, Bluetooth (BT), and BLE capabilities, along with support for the real-time operating system FreeRTOS. This project leverages these features to create a robust BLE communication system that can handle data packetization and multitasking without compromising performance.

### Requirements

- **Hardware**: ESP32 microcontroller.
- **Software**: 
  - ESP-IDF framework.
  - NimBLE library for BLE operations.
  - FreeRTOS (comes with ESP-IDF).
  - Arduino core for ESP32 (optional, for Arduino IDE users).

Note:  this was built under platformio esp32 arduino framework, but should run nicely under esp-idf framework (dont forget to enable BLE and NimBLE in menuconfig).

### How It Works

The main components of this project are:

- **BLE Communication**: Utilizes the NimBLE library to establish and manage BLE connections. The `NuPacket` class abstracts the packetization and data handling over BLE.
- **FreeRTOS Tasks**: Two tasks are created for handling RX (receive) and TX (transmit) operations independently. This separation ensures that receiving data from one BLE device and transmitting data to another (or the same) device can occur simultaneously without blocking each other.
- **FreeRTOS Queues**: Queues are used to safely pass data between the RX and TX tasks. This mechanism allows for decoupling the tasks, making the system more modular and responsive.

#### Why Use Tasks and Queues?

- **Concurrency**: Tasks allow the ESP32 to perform multiple operations concurrently. This is crucial for maintaining a responsive BLE communication channel while performing other operations.
- **Decoupling**: By separating the RX and TX operations into different tasks, the system's design becomes more modular, allowing for easier maintenance and scalability.
- **Data Integrity**: Queues provide a thread-safe way to transfer data between tasks, ensuring that packets are not lost or corrupted when the system is under load.
- **Efficiency**: This approach allows for efficient use of the ESP32's dual-core processor, distributing the workload to improve performance and responsiveness.

### Setup and Usage

1. **Environment Setup**: Ensure the ESP-IDF framework is installed and configured for your development environment. If using the Arduino IDE, install the ESP32 core and the NimBLE library.
2. **Hardware Setup**: Connect your ESP32 to your computer via USB.
3. **Project Configuration**: Clone this repository and open the project in your IDE or editor of choice.
4. **Build and Flash**: Compile the project and flash it to your ESP32 device.
5. **Monitor**: Use a serial monitor to view log messages and observe the BLE communication in action.

### Contributing

Contributions to this project are welcome. Please feel free to fork the repository, make your changes, and submit a pull request.

### License

This project is licensed under the Creative Commons Attribution 4.0 International (CC BY 4.0) license.

### Acknowledgments

- Ángel Fernández Pineda his great library used on this project.
- The ESP-IDF and NimBLE teams for providing the foundational libraries used in this project.