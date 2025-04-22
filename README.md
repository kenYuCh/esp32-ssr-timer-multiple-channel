| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |

# _Sample project_

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This is the simplest buildable example. The example is used by command `idf.py create-project`
that copies the project to user specified path and set it's name. For more information follow the [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project)



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
# esp32-ssr-timer-multiple-channel


There are four relays in total that can control 110~220VAC 3~32VDC.

Tools: ESP32 s3, 4-Channel Relay Module,

Communication: MQTT

illustrate:
Each Channel has three operating periods, and each period cannot be repeated.

There will be a start time and a stop time that can be operated, and parameters such as the running seconds, interval seconds, and the number of runs can be set.
<img width="875" alt="截圖 2025-03-08 中午12 05 16" src="https://github.com/user-attachments/assets/63b1dfb2-968f-443e-973e-697ee578451a" />

## MQTT operations:
'''
// period-1
{
	"id":	"0",
	"sn":	"123",
	"label":	"switch valve",
	"label_en": "multiple channels",
	"ctrl":	"2-1",
	"params":	[
        {"name": "st_time", "value": "00:001"},
        {"name": "sp_time", "value": "08:59"},
        {"name": "on_dur", "value": "30"},
        {"name": "intv_dur", "value": "10"},
        {"name": "cnt", "value": "4002"},
        {"name": "period_en", "value": "1"},
        {"name": "mode", "value": "2"}
	]
}
// period-2
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
// period-3
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
'''
