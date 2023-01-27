# Software setup

SEGGER Embedded Studio
https://www.segger.com/downloads/embedded-studio/

nRF5 SDK
https://www.nordicsemi.com/Products/Development-software/nRF5-SDK/Download#infotabs

# Getting example app to build
In nRF52 SDK examples/ble_peripheral/ble_app_beacon for pca10040 platform using SEGGER ES
 - exclude SEGGER_RTT_Syscalls_SES.c from build
    https://devzone.nordicsemi.com/f/nordic-q-a/85405/nrf5-sdk-17-1-0-examples-is-not-compiling-in-latest-ses-6-20a
 - .text and .rodata size in flash_placement.xml need to be fixed
    https://devzone.nordicsemi.com/f/nordic-q-a/89236/build-error/374042


