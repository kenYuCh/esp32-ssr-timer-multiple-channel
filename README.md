| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |

# ESP32 SSR Timer Multiple Channel

## How to use example
We encourage the users to use the example as a template for the new projects.
A recommended way is to follow the instructions on a [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project).

## Example folder contents

The project **sample_project** contains one source file in C language [main.c](main/main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md                  This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.


## Overview
This project involves controlling a 4-Channel Relay Module using an ESP32-S3 microcontroller. The relays can handle 110~220VAC or 3~32VDC. The system communicates via MQTT to manage multiple channels, each with three non-overlapping operating periods. Each period includes configurable parameters such as start time, stop time, running duration, interval duration, and number of runs.

## Hardware
- **Microcontroller**: ESP32-S3
- **Relay Module**: 4-Channel Relay Module (compatible with 110~220VAC, 3~32VDC)
- **Power Supply**: Ensure proper voltage for ESP32-S3 and relay module
- **Network**: Wi-Fi for MQTT communication
- **NVS**: The nvs mechanism is used. All setting parameters will be permanently stored in the device.

## Software Requirements
- **Platform**: ESP32-IDF
- **Libraries**:
  - mqtt (for MQTT)
  - WiFi (for ESP32 Wi-Fi connectivity)
  - cJSON (for parsing MQTT JSON payloads)
- **MQTT Broker**: e.g., Mosquitto, HiveMQ, or any cloud-based MQTT service

## System Description
Each of the four relay channels can be independently controlled. Each channel supports **three operating periods** per day, and these periods must not overlap. The parameters for each period include:
- **Start Time (`st_time`)**: When the period begins (e.g., "00:01").
- **Stop Time (`sp_time`)**: When the period ends (e.g., "08:59").
- **On Duration (`on_dur`)**: Duration the relay is ON in seconds (e.g., "30").
- **Interval Duration (`intv_dur`)**: Time between ON cycles in seconds (e.g., "10").
- **Count (`cnt`)**: Number of ON/OFF cycles (e.g., "4002").
- **Period Enable (`period_en`)**: Enable/disable period (1 = enabled, 0 = disabled).
- **Mode (`mode`)**: Operation mode (e.g., "2" for timer-based control).
- !!! ctrl: channelId-periodId

## MQTT Communication
The system uses MQTT to receive configuration commands. Each message is a JSON payload specifying the channel, period, and parameters. Below are example payloads for a single channel's three periods:
[ Reserve one channel (CH5) ]
<img width="875" alt="截圖 2025-03-08 中午12 05 16" src="https://github.com/user-attachments/assets/02556598-5b79-4c7e-abc5-24cf3ce75077" />

### Period 1
```json
{
	"id":	"0",
	"sn":	"123",
	"label":	"switch valve",
	"label_en": "multiple channels",
	"ctrl":	"2-1",
	"params":	[
		{"name": "st_time", "value": "00:01"},
		{"name": "sp_time", "value": "08:59"},
		{"name": "on_dur", "value": "30"},
		{"name": "intv_dur", "value": "10"},
		{"name": "cnt", "value": "4002"},
		{"name": "period_en", "value": "1"},
		{"name": "mode", "value": "2"}
	]
}
```

### Period 2
```json
{
	"id":	"0",
	"sn":	"123",
	"label":	"switch valve",
	"label_en": "multiple channels",
	"ctrl":	"2-2",
	"params":	[
		{"name": "st_time", "value": "10:33"},
		{"name": "sp_time", "value": "14:59"},
		{"name": "on_dur", "value": "30"},
		{"name": "intv_dur", "value": "10"},
		{"name": "cnt", "value": "4002"},
		{"name": "period_en", "value": "1"},
		{"name": "mode", "value": "2"}
	]
}
```

### Period 3
```json
{
	"id":	"0",
	"sn":	"123",
	"label":	"switch valve",
	"label_en": "multiple channels",
	"ctrl":	"2-3",
	"params":	[
		{"name": "st_time", "value": "17:33"},
		{"name": "sp_time", "value": "23:59"},
		{"name": "on_dur", "value": "30"},
		{"name": "intv_dur", "value": "10"},
		{"name": "cnt", "value": "4002"},
		{"name": "period_en", "value": "1"},
		{"name": "mode", "value": "2"}
	]
}
```

### MQTT Topics
- **Subscribe Topic**: `device/<sn>/control` (e.g., `device/123/control`)
  - The ESP32 subscribes to this topic to receive configuration updates.
- **Publish Topic**: `device/<sn>/status` (e.g., `device/123/status`)
  - The ESP32 publishes relay status or confirmation of received commands.

## Setup Instructions
1. **Hardware Setup**:
   - Connect the 4-Channel Relay Module to the ESP32-S3 GPIO pins (e.g., GPIO 4, 5, 6, 7 for relays 1-4).
   - Power the ESP32-S3 and relay module appropriately.
   - Ensure the relay module is compatible with the load (110~220VAC or 3~32VDC).

2. **Software Setup**:
   - Install the required libraries in your development environment.
   - Configure Wi-Fi credentials and MQTT broker details in the code.
   - Parse incoming JSON MQTT messages to extract period parameters.

3. **MQTT Broker**:
   - Set up an MQTT broker (local or cloud-based).
   - Ensure the ESP32 can connect to the broker over Wi-Fi.

## Operation
- The ESP32 listens for MQTT messages on the subscribed topic.
- When a valid JSON payload is received, it updates the corresponding channel and period settings.
- The system checks the current time against each period’s start and stop times.
- If within a period and enabled, the relay operates based on the `on_dur`, `intv_dur`, and `cnt` parameters.
- Status updates (e.g., relay ON/OFF) are published to the status topic.

## Example Workflow
1. An MQTT message is sent to `device/123/control` with the Period 1 configuration for Channel 2.
2. The ESP32 parses the JSON, validates the `ctrl` field (e.g., "2-1" for Channel 2, Period 1), and stores the parameters.
3. Between 00:01 and 08:59, Channel 2’s relay turns ON for 30 seconds, OFF for 10 seconds, repeating 4002 times or until the stop time is reached.
4. The ESP32 publishes a status update to `device/123/status` confirming the operation.

## Notes
- Ensure periods do not overlap to avoid conflicts (e.g., Period 1 ends at 08:59, Period 2 starts at 10:33).
- Validate MQTT payloads to prevent invalid configurations.

## Future Improvements
- Add local storage (e.g., SPIFFS) to save configurations during power loss.
- Implement a web interface for manual configuration.
- Support dynamic channel and period allocation via MQTT.
