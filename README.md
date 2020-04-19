# MCODE (Mini-CODE) project
This is a MIT-licensed, simple OS for small devices like AVR8 or ARM Cortex-3.
It includes a lot of ready-to-use pieces of code for easy prototyping, including:
* Event handling scheduler;
* Millisecond event handling scheduler;
* Timer (possibly periodic) with millisecond resolution;
* Printing utilities for formatted output with the possibility of redirection;
* Advanced String Parsing utilities;
* GSM module Engine;
* Highly configurable and extendable Command Engine available in Serial Console and in GSM Console;
* Command engine modes: (passwd protected) super-user/user/advanced-user (non-passwd protected);
* Simple programming interface with user Variable (types: String, Integer, NVM_Integer);
* Supported targets:
  * STM32 targets;
  * AVR targets;
  * EMU (x86_64) host target for easy simulation;
  * GTest/CUnit targets for Unit Tests with Code Coverage support (GTest only);
* HW support:
  * UART: up to 2 interfaces (one of which with possible connection to a GSM Module);
  * I80 interface for LCD modules;
  * Generic LCD interface;
  * NVM interface;
  * RTC interface;
  * SPI interface;
  * GPIO/LED interface;
  * PWM/SOUND interface;
  * TWI interface for interfacing external I2C devices;
  * Watchdod support;
  * Time management engine;
  * IR support (currently incomplete);
* LCD console implementation with support for quite some escape sequences;
* Graphical phonts for LCD console;
* Basic security support:
  * sha256 hash support (for now, this is the only security-related functionality);
* AVR109-compatible AVR Bootloader;

## Getting Started
The easiest target to try is EMU (as it does not depend on any HW).

### Development Environment
I mostly use the following system for development:
* Ubuntu 18.04.4 LTS

### Installing Build Dependencies for EMU Target
The following packages required for building code for EMU Target:
* git/gcc/cmake/make and other usual develpment packages;
* QT4 development packages (for LCD Simulator);
* SQLite Development package (Persistent Storage implementation for EMU Target);
* glib2.0 Development package (dependency for SQLite Development package);

### Build steps for EMU target
Here are the steps to build the application for EMU target:
* $ cd good-directory-for-building/
* $ git https://w5292c@github.com/w5292c/mcode.git
* $ cd mcode/
* $ mkdir build/
* $ cd build/
* $ cmake ../targets/pc/
* $ make
After these steps, the following executable are created:
* console-test: the Main Application that accepts commands in the console;
* mcode-simulator: the Modem Simulator, the Main Application can communicate with it;
The both applications should be launched and they will communicate with each other.
In the main application, the following commands can be launched first (in the application console):
* $ help
* >>> Supported commands: [supported-commands-list]
* $ help help
* >>> Help test for specific command, 'help' in our case.
The Modem Simulator also supports console, it supports different list of commands,
'help' is supported. The old Command Engine supports additional commands, they can
be inspected with the 'help-old' command.
Use 'poweroff' command (in the application console) to exit the Main and Modem Simulator apps.

### Build steps for GTest/CUnit targets
The code supports GTest (new) and CUnit (old, can be removed soon) targets for Unit Tests.
Here are the steps for building for GTest/CUnit:
* $ # git-cloning and build directory creating is the same as for 'EMU',
* $ # so we are supposed to be in mcode/build directory initially
* $ cmake ../targets/cunit/ # Use this command to configure for CUnit target OR
* $ cmake ../targets/gtest/ # Use this command to configure for GTest target INSTEAD;
* $ make
* $ make cov # This is supported in GTest target, this creates 'html/index.html' with coverage

### Installing Build Dependencies for AVR Target
In order to build for AVR target, the corresponding tools should be installed, like
'avr-gcc/avr-ld/etc' including the AVR standard library,
here are the intalled packages in my system:
* avr-libc     # Standard library
* avrdude      # Flashing HW
* binutils-avr # Binutils for AVR
* gcc-avr      # GCC
* gdb-avr      # Debugging
Custom HW boards I am using (based on ATMega32A) are descibed here:
* targets/avr/README
* targets/avr-tv/README

### Build steps for AVR target
Here are the steps:
* $ # git-cloning and build directory creating is the same as for 'EMU',
* $ # so we are supposed to be in mcode/build directory initially
* $ cmake ../targets/avr-tv/ # Custom ATMega32A board without LCD OR
* $ cmake ../targets/avr/    # Custom ATMega32A board with LCD INSTEAD;
* $ make

### Deployment steps for AVR target
If the bootloader is already in place, one can just launch the bootloader
(command 'bootloader' on UART port) and use the following command:
$ avrdude -b 115200 -c avr109 -p m32 -P /dev/ttyUSB0 -U flash:w:console-test.flash.hex -U eeprom:w:console-test.eeprom.hex
More informatin can be found in:
* targets/avr/README
* targets/avr-tv/README

### Installing Build Dependencies for STM32 Target
For STM32 Target the ARM toolchains/standard library are required to be installed.
Also, the following 2 components are required:
* stm32-spd
* stm32-cmake
The can be built using the following scipt and docfile:
* targets/stm32-common/README
* targets/stm32-common/build-stm32-cmake.sh

### Build steps for STM32 targets
See the following file for details:
* targets/stm32/README

### Deployment steps for STM32 target
See the following file for details:
* targets/stm32/README

## License
This project is licensed under the MIT License - see the LICENSE file for details
