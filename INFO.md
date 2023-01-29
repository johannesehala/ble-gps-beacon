#SEGGER Embedded Studio
https://www.segger.com/downloads/embedded-studio/

#nRF5 SDK
https://www.nordicsemi.com/Products/Development-software/nRF5-SDK/Download#infotabs

# Getting example app to build
In nRF52 SDK examples/ble_peripheral/ble_app_beacon for pca10040 platform using SEGGER ES
 - exclude SEGGER_RTT_Syscalls_SES.c from build
    https://devzone.nordicsemi.com/f/nordic-q-a/85405/nrf5-sdk-17-1-0-examples-is-not-compiling-in-latest-ses-6-20a
 - .text and .rodata size in flash_placement.xml need to be fixed
    https://devzone.nordicsemi.com/f/nordic-q-a/89236/build-error/374042

#Nordic development kit PCA number and chip
https://infocenter.nordicsemi.com/index.jsp?topic=%2Fug_gsg_ses%2FUG%2Fgsg%2Fchips_and_sds.html

#JLink RTT Nordic 
https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/debugging-with-real-time-terminal

#LED Button example application note
https://infocenter.nordicsemi.com/pdf/nan_36.pdf?cp=16_3
 - set device name - sd_ble_gap_device_name_set() in ble_gap.h
 - set up advertising data ble_advdata_set() in ble_advdata.h sd_ble_gap_adv_data_set in ble_gap.h
 - connection parameters ble_conn_params_init() in ble_conn_params.h sd_ble_gap_ppcp_set() in ble_gap.h
 - start advertising sd_ble_gap_adv_start() in ble_gap.h 
 
 - advertising structure types
    - ble_gap_conn_sec_mode_t (ble_gap.h) 
    - ble_advdata_t (ble_advdata.h)
    - ble_gap_adv_params_t (are passed to softdevice with sd_ble_gap_adv_start())

#BLUETOOTH LE quick overview
Using advertising GAP generic access profile (parameters that govern advertising and connection among other things)
 - advertising interval 20 ms - 10.24 sec
 - packet 31 bytes
 - advertising is done on 3 frequencies

GATT generic attribute profile - for data value transfer
 - GATT server has data, GATT client wants data
 - attributes are objects that contain the actual data, there is a handle, uuid, operation (r/w etc.) and value associated with an attribute
 - a characteristic contains two or more attributes, keep similar attributes in one characteristic?
 - characteristic can have describtors, these are like attributes but differ in that they hold no value?
 - a servis is one or more characteristics
 - A profile can be defined to collect one or more services into a use case description

#Bluetooth SIG GATT attribute UUID list (16-bit IDs)
https://specificationrefs.bluetooth.com/assigned-values/16-bit%20UUID%20Numbers%20Document.pdf
Common Data Types
https://btprodspecificationrefs.blob.core.windows.net/assigned-numbers/Assigned%20Number%20Types/Assigned%20Numbers.pdf 

#A Nordic SoftDevice 
is compiled binary code (these seem to be a part of nRF52 SDK, so no need to download separately)
 - these are hardware specific
 - S:a:b:c:
 - a - protocol (1 - BLE, 2 - ANT, 3 - BLE&ANT)
 - b - master or slave (1 - only peripheral, 2 - central only, 3 - both)
 - c - chip/hardware (2 - nrf52832)

