/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_app_beacon_main main.c
 * @{
 * @ingroup ble_sdk_app_beacon
 * @brief Beacon Transmitter Sample Application main file.
 *
 * This file contains the source code for an Beacon transmitter sample application.
 */

#include <stdbool.h>
#include <stdint.h>
#include "nordic_common.h"
#include "bsp.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advdata.h"
#include "app_timer.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "SEGGER_RTT.h"

#define BLE_DEVICE_NAME                 "GPS Beacon Test"
#define HC_GPS_STRING_LENGTH            25
#define MAX_GPS_STRING_LENGTH           26

#define APP_BLE_CONN_CFG_TAG            1                                  /**< A tag identifying the SoftDevice BLE configuration. */

#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(100, UNIT_0_625_MS)  /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */

#define APP_BEACON_INFO_LENGTH          0x17                               /**< Total length of information advertised by the Beacon. */
#define APP_ADV_DATA_LENGTH             0x15                               /**< Length of manufacturer specific data in the advertisement. */
#define APP_DEVICE_TYPE                 0x02                               /**< 0x02 refers to Beacon. */
#define APP_MEASURED_RSSI               0xC3                               /**< The Beacon's measured RSSI at 1 meter distance in dBm. */
#define APP_COMPANY_IDENTIFIER          0x0059                             /**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */
#define APP_MAJOR_VALUE                 0x01, 0x02                         /**< Major value used to identify Beacons. */
#define APP_MINOR_VALUE                 0x03, 0x04                         /**< Minor value used to identify Beacons. */
#define APP_BEACON_UUID                 0x01, 0x12, 0x23, 0x34, \
                                        0x45, 0x56, 0x67, 0x78, \
                                        0x89, 0x9a, 0xab, 0xbc, \
                                        0xcd, 0xde, 0xef, 0xf0            /**< Proprietary UUID for Beacon. */

#define DEAD_BEEF                       0xDEADBEEF                         /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#if defined(USE_UICR_FOR_MAJ_MIN_VALUES)
#define MAJ_VAL_OFFSET_IN_BEACON_INFO   18                                 /**< Position of the MSB of the Major Value in m_beacon_info array. */
#define UICR_ADDRESS                    0x10001080                         /**< Address of the UICR register used by this example. The major and minor versions to be encoded into the advertising data will be picked up from this location. */
#endif

static ble_gap_adv_params_t m_adv_params;                                  /**< Parameters to be passed to the stack when starting advertising. */
static uint8_t              m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET; /**< Advertising handle used to identify an advertising set. */
static uint8_t              m_enc_advdata_ping[BLE_GAP_ADV_SET_DATA_SIZE_MAX];  /**< Buffer for storing an encoded advertising set. */
static uint8_t              m_enc_advscan_ping[BLE_GAP_ADV_SET_DATA_SIZE_MAX];  /**< Buffer for storing an encoded advertising set. */
static uint8_t              m_enc_advdata_pong[BLE_GAP_ADV_SET_DATA_SIZE_MAX];  /**< Buffer for storing an encoded advertising set. */
static uint8_t              m_enc_advscan_pong[BLE_GAP_ADV_SET_DATA_SIZE_MAX];  /**< Buffer for storing an encoded advertising set. */

/**@brief Struct that contains pointers to the encoded advertising data. */
static ble_gap_adv_data_t m_adv_data_ping =
{
    .adv_data =
    {
        .p_data = m_enc_advdata_ping,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    },
    .scan_rsp_data =
    {
        .p_data = m_enc_advscan_ping,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX

    }
};

/**@brief Struct that contains pointers to the encoded advertising data. */
static ble_gap_adv_data_t m_adv_data_pong =
{
    .adv_data =
    {
        .p_data = m_enc_advdata_pong,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    },
    .scan_rsp_data =
    {
        .p_data = m_enc_advscan_pong,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX

    }
};


static uint8_t m_beacon_info[APP_BEACON_INFO_LENGTH] =                    /**< Information advertised by the Beacon. */
{
    APP_DEVICE_TYPE,     // Manufacturer specific information. Specifies the device type in this
                         // implementation.
    APP_ADV_DATA_LENGTH, // Manufacturer specific information. Specifies the length of the
                         // manufacturer specific data in this implementation.
    APP_BEACON_UUID,     // 128 bit UUID value.
    APP_MAJOR_VALUE,     // Major arbitrary value that can be used to distinguish between Beacons.
    APP_MINOR_VALUE,     // Minor arbitrary value that can be used to distinguish between Beacons.
    APP_MEASURED_RSSI    // Manufacturer specific information. The Beacon's measured TX power in
                         // this implementation.
};

static uint8_t gps_string[HC_GPS_STRING_LENGTH] =
{
  0x35,
  0x39,
  0x2a,
  0x32,
  0x36,
  0x27,
  0x31,
  0x32,
  0x2e,
  0x36,
  0x22,
  0x4e,
  0x20,
  0x32,
  0x34,
  0x2a,
  0x34,
  0x34,
  0x27,
  0x33,
  0x34,
  0x2e,
  0x30,
  0x22,
  0x45
};

static uint8_t gps_string_2[HC_GPS_STRING_LENGTH] =
{
  0x32,
  0x36,
  0x2a,
  0x31,
  0x32,
  0x27,
  0x30,
  0x35,
  0x2e,
  0x34,
  0x22,
  0x53,
  0x20,
  0x32,
  0x38,
  0x2a,
  0x30,
  0x31,
  0x27,
  0x35,
  0x32,
  0x2e,
  0x37,
  0x22,
  0x45
};
static uint8_t user_string[MAX_GPS_STRING_LENGTH]; //Max possible GPS characters is 26 for format 36??06'46.8"N 115??10'23.2"W

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static bool advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata, advscan;
    uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

    ble_advdata_manuf_data_t manuf_specific_data;
    ble_gap_conn_sec_mode_t gap_sec_mode;

    manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;

#if defined(USE_UICR_FOR_MAJ_MIN_VALUES)
    // If USE_UICR_FOR_MAJ_MIN_VALUES is defined, the major and minor values will be read from the
    // UICR instead of using the default values. The major and minor values obtained from the UICR
    // are encoded into advertising data in big endian order (MSB First).
    // To set the UICR used by this example to a desired value, write to the address 0x10001080
    // using the nrfjprog tool. The command to be used is as follows.
    // nrfjprog --snr <Segger-chip-Serial-Number> --memwr 0x10001080 --val <your major/minor value>
    // For example, for a major value and minor value of 0xabcd and 0x0102 respectively, the
    // the following command should be used.
    // nrfjprog --snr <Segger-chip-Serial-Number> --memwr 0x10001080 --val 0xabcd0102
    uint16_t major_value = ((*(uint32_t *)UICR_ADDRESS) & 0xFFFF0000) >> 16;
    uint16_t minor_value = ((*(uint32_t *)UICR_ADDRESS) & 0x0000FFFF);

    uint8_t index = MAJ_VAL_OFFSET_IN_BEACON_INFO;

    m_beacon_info[index++] = MSB_16(major_value);
    m_beacon_info[index++] = LSB_16(major_value);

    m_beacon_info[index++] = MSB_16(minor_value);
    m_beacon_info[index++] = LSB_16(minor_value);
#endif

    manuf_specific_data.data.p_data = (uint8_t *) gps_string;
    manuf_specific_data.data.size   = HC_GPS_STRING_LENGTH;

    // Set device name and connection security mode
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&gap_sec_mode);
    
    err_code = sd_ble_gap_device_name_set(&gap_sec_mode, (const uint8_t *)BLE_DEVICE_NAME, strlen(BLE_DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));
    memset(&advscan, 0, sizeof(advscan));

    advdata.name_type             = BLE_ADVDATA_FULL_NAME;
    advdata.flags                 = flags;
    //advdata.p_manuf_specific_data = &manuf_specific_data;

    advscan.p_manuf_specific_data = &manuf_specific_data;

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED;
    m_adv_params.p_peer_addr     = NULL;    // Undirected advertisement.
    m_adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval        = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.duration        = 0;       // Never time out.

    err_code = ble_advdata_encode(&advdata, m_adv_data_ping.adv_data.p_data, &m_adv_data_ping.adv_data.len);
    APP_ERROR_CHECK(err_code);

    // Also encode adv data for pong, cuz this is static.
    err_code = ble_advdata_encode(&advdata, m_adv_data_pong.adv_data.p_data, &m_adv_data_pong.adv_data.len);
    APP_ERROR_CHECK(err_code);

    err_code = ble_advdata_encode(&advscan, m_adv_data_ping.scan_rsp_data.p_data, &m_adv_data_ping.scan_rsp_data.len);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data_ping, &m_adv_params);
    APP_ERROR_CHECK(err_code);

    return true; //Pong is available, cuz ping was used.
}


/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    ret_code_t err_code;

    err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing logging. */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for initializing LEDs. */
static void leds_init(void)
{
    ret_code_t err_code = bsp_init(BSP_INIT_LEDS, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing timers. */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(bool p)
{
  uint32_t i;
  bool pong = p; //Indicates if pong is available or not
  uint32_t err_code;
  char c;
  ble_advdata_t advscan;
  ble_advdata_manuf_data_t manuf_specific_data;

  manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;
  manuf_specific_data.data.p_data = (uint8_t *) user_string;
  
  SEGGER_RTT_WriteString(0, "A - start token, indicates beginning of new string.\n");
  SEGGER_RTT_WriteString(0, "B - end token, indicates end of string.\n");
  SEGGER_RTT_WriteString(0, "Max string length is 26 characters.\n");
  SEGGER_RTT_WriteString(0, "Characters are accepted until buffer is full (26 bytes) or B is received\n");
  SEGGER_RTT_WriteString(0, "Accepts most UTF8 characters.\n");
  SEGGER_RTT_WriteString(0, "Copy-paste works up to 15 characters, the rest is dropped, program not fast enough.\n");
  SEGGER_RTT_WriteString(0, "Suggested GPS format - 36*06'46.8\"N 115*10'23.2\"W\n");
  
  for(;;)
  {
    c = SEGGER_RTT_WaitKey(); // will block until data is available
    if(c == 'A')
    {
      SEGGER_RTT_WriteString(0, "\n\nStarted receiving..\n\n");
      i=0;
      while(i < 26 && c != 'B')
      {
        c = SEGGER_RTT_WaitKey(); // will block until data is available
        if(c >= 0x20 && c <= 0x7e)user_string[i++] = c;
      }
      SEGGER_RTT_printf(0, "\n\nString received. Len: %d\n",i);
    
      // Encode and publish.
      if(c == 'B')manuf_specific_data.data.size = i-1;
      else manuf_specific_data.data.size = i;

      if(pong)
      {
        memset(&advscan, 0, sizeof(advscan));
        advscan.p_manuf_specific_data = &manuf_specific_data;

        m_adv_data_pong.scan_rsp_data.len = BLE_GAP_ADV_SET_DATA_SIZE_MAX;
        err_code = ble_advdata_encode(&advscan, m_adv_data_pong.scan_rsp_data.p_data, &m_adv_data_pong.scan_rsp_data.len);
        APP_ERROR_CHECK(err_code);

        err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data_pong, NULL);
        APP_ERROR_CHECK(err_code);

        pong = false;
      }
      else //must be ping
      {
        memset(&advscan, 0, sizeof(advscan));
        advscan.p_manuf_specific_data = &manuf_specific_data;

        m_adv_data_ping.scan_rsp_data.len = BLE_GAP_ADV_SET_DATA_SIZE_MAX;
        err_code = ble_advdata_encode(&advscan, m_adv_data_ping.scan_rsp_data.p_data, &m_adv_data_ping.scan_rsp_data.len);
        APP_ERROR_CHECK(err_code);

        err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data_ping, NULL);
        APP_ERROR_CHECK(err_code);

        pong = true;
      }
      nrf_delay_ms(500); // Wait a little
    }
  }
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    bool pong; //Indicates if pong is available or not
    // Initialize.
    log_init();
    timers_init();
    leds_init();
    power_management_init();
    ble_stack_init();
    pong = advertising_init();

    // Start execution.
    NRF_LOG_INFO("Beacon example started.");
    advertising_start();

    // Enter main loop.
    for (;; )
    {
        idle_state_handle(pong);
    }
}


/**
 * @}
 */
