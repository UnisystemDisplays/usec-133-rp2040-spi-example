OVERVIEW
--------

This repository contains example of [*usec-133-generic-spi-lib*](https://github.com/UnisystemDisplays/usec-133-generic-spi-lib) library integration for [*Raspberry Pi Pico (RP2040)*](https://www.raspberrypi.com/products/raspberry-pi-pico/) platform. Please check [*usec-133-generic-spi-lib*](https://github.com/UnisystemDisplays/usec-133-generic-spi-lib) project repository for more info about library itself.

PREREQUISITES
-------------

To get started, please make sure, that all libraries and tools for C/C++ development on RP2040 microcontrollers (recommended usage of *Raspberry Pi Pico VS Code Extension*) are properly installed and configured on your machine - visit [*Getting started with Raspberry Pi Pico*](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) for more info.

CONNECTIONS
-----------

|  USEC module connector | Raspberry Pi Pico |
| ---- | ----- |
| VDD  | 3.3V  |
| GND  | GND   |
| SCLK | 18    |
| MISO | 16    |
| MOSI | 19    |
| CS   | 17    |
| RDY  | 20    |
| RST  | 21    |

__Note:__ *you can easily reconfigure pinout by editting platform.h file*

COMPILATION
-----------

[1] Clone *usec-133-rp2040-spi-example* repository:

`git clone https://github.com/UnisystemDisplays/usec-133-rp2040-spi-example.git`

[2] Open project with *Visual Studio Code* with installed *Raspberry Pi Pico VS Code Extension*

[3] Compile project and load *usec-133-rp2040-spi-example.elf* file directly from *Visual Studio Code* or *usec-133-rp2040-spi-example.uf2* file via drag and drop method.

GETTING HELP
------------

Please contact Unisystem support - [*<lukasz.skalski@unisystem.com>*](lukasz.skalski@unisystem.com) or [*<jacek.marcinkowski@unisystem.com>*](jacek.marcinkowski@unisystem.com)

LICENSE
-------

See *LICENSE.txt* file for details.
