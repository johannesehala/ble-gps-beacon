# Software setup

* SEGGER Embedded Studio v7.10a
https://www.segger.com/downloads/embedded-studio/

* nRF5 SDK v17.1.0
https://www.nordicsemi.com/Products/Development-software/nRF5-SDK/Download#infotabs

# Usage
- Tested on nRF52840 DK
- add project to SDK tree at root
- open project in SEGGER Embedded Studio 
   - comodule_ha/ble_gps_beacon/pca10056/s140/ses/*.emProject
   - for other platforms (and IDE's) there may be compiling issues
- once the program is compiled and running open the debug terminal and give input through the terminal:
   - A - start token, indicates beginning of new string.
   - B - end token, indicates end of string.
   - Max string length is 26 characters.
   - Characters are accepted until buffer is full (26 bytes) or B is received. Only then is the string transmitted as advertising data.
   - Accepts most UTF8 characters.
   - Suggested GPS format - 36*06'46.8"N 115*10'23.2"W
- In nRF Connect App change the manufacturer data display coding to UTF8.

# Compiling issues with SEGGER ES v7.10a and nRF5 SDK v17.1.0
There were two issues when compiling nRF52 SDK examples/ble_peripheral/ble_app_beacon for pca10040 platform using SEGGER ES
- exclude SEGGER_RTT_Syscalls_SES.c from build
    https://devzone.nordicsemi.com/f/nordic-q-a/85405/nrf5-sdk-17-1-0-examples-is-not-compiling-in-latest-ses-6-20a
- .text and .rodata size in flash_placement.xml need to be fixed
    https://devzone.nordicsemi.com/f/nordic-q-a/89236/build-error/374042
