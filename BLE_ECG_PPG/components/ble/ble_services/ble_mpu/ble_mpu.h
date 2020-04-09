#ifndef BLE_LBS_H__
#define BLE_LBS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

#define LBS_UUID_BASE        {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, \
                              0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}
#define MAX_UUID_SERVICE     0x1526
#define MAX_UUID_MPU_CHAR    0x1527

// Forward declaration of the ble_lbs_t type. 
typedef struct ble_mpus_s ble_mpus_t;

typedef void (*ble_lbs_led_write_handler_t) (ble_mpus_t * p_lbs, uint8_t new_state);
															
typedef struct
{
    ble_lbs_led_write_handler_t led_write_handler; /**< Event handler to be called when the LED Characteristic is written. */
} ble_mpus_init_t;

/**@brief mpu6050 Service structure. This structure contains various status information for the service. */
struct ble_mpus_s
{
    uint16_t                    service_handle;      /**< Handle of Mpu6050 Service (as provided by the BLE stack). */
	  //ble_gatts_char_handles_t    led_char_handles;    /**< Handles related to the LED Characteristic. */
    ble_gatts_char_handles_t    mpu_char_handles;    /**< Handles related to the mpu Characteristic. */
	  uint8_t                     uuid_type;           /**< UUID type for the LED Button Service. */
    uint16_t                    conn_handle;         /**< Handle of the current connection (as provided by the BLE stack). BLE_CONN_HANDLE_INVALID if not in a connection. */
	  //ble_lbs_led_write_handler_t led_write_handler;   /**< Event handler to be called when the LED Characteristic is written. */
};					
typedef struct
{
    uint8_t cnt;
	  uint8_t buf[8]; 
}adc_value_t;
typedef struct
{
    uint8_t cnt;
	  uint8_t buf[12]; 
}max_value_t;
															
uint32_t ble_mpus_init(ble_mpus_t * p_mpus, const ble_mpus_init_t * p_mpus_init);
uint16_t ble_mpu6050_on_data_read(ble_mpus_t * p_mpus, int16_t *pAVal, int16_t *pGVal);
uint16_t ble_max30102_on_data_read(ble_mpus_t * p_mpus, uint8_t *temp,uint8_t count, bool flag, uint8_t *sensor_val);
void ble_mpus_on_ble_evt(ble_mpus_t * p_lbs, ble_evt_t * p_ble_evt);
#endif
