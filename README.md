# Software setup

* SEGGER Embedded Studio v7.10a
https://www.segger.com/downloads/embedded-studio/

* nRF5 SDK v17.1.0
https://www.nordicsemi.com/Products/Development-software/nRF5-SDK/Download#infotabs

# Usage
- add solution project to SDK tree at root
- open project in SEGGER Embedded Studio comodule_ha/ble_gps_beacon/pca10056/s140/ses/*.emProject
- once the program is compiled and running open the debug terminal and give input through the terminal:
   - A - start token, indicates beginning of new string.
   - B - end token, indicates end of string.
   - Max string length is 26 characters.
   - Characters are accepted until buffer is full (26 bytes) or B is received. Only then is the string transmitted as advertising data.
   - Accepts most UTF8 characters.
   - Suggested GPS format - 36*06'46.8"N 115*10'23.2"W
- In nRF Connect App change the manufacturer data display coding to UTF8.

