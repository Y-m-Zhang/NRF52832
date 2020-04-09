#include "ble_mpu.h"
#include "ble_srv_common.h"
#include "sdk_common.h"

/**@brief Function for handling the Connect event.
 *
 * @param[in] p_lbs      LED Button Service structure.
 * @param[in] p_ble_evt  Event received from the BLE stack.
 */
static void on_connect(ble_mpus_t * p_lbs, ble_evt_t * p_ble_evt)
{
    p_lbs->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in] p_lbs      LED Button Service structure.
 * @param[in] p_ble_evt  Event received from the BLE stack.
 */
static void on_disconnect(ble_mpus_t * p_lbs, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_lbs->conn_handle = BLE_CONN_HANDLE_INVALID;
}

void ble_mpus_on_ble_evt(ble_mpus_t * p_lbs, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_lbs, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_lbs, p_ble_evt);
            break;
            
        case BLE_GATTS_EVT_WRITE:
            //on_write(p_lbs, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}
/**********************************************************************************************
 * 描  述 : 添加特性
 * 参  数 : p_lbs[in]:指向thi服务结构体
 *          p_lbs_init[in]:thi服务初始化结构体
 * 返回值 : 成功返回NRF_SUCCESS，否则返回错误代码
 ***********************************************************************************************/ 
static uint32_t mpu_char_add(ble_mpus_t * p_lbs, const ble_mpus_init_t * p_lbs_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    
    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.read = 1;
	  char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
	  char_md.p_cccd_md         = NULL;
    char_md.p_cccd_md         = &cccd_md;

    ble_uuid.type = p_lbs->uuid_type;
    ble_uuid.uuid = MAX_UUID_MPU_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;

		attr_char_value.init_len     = 20;
    attr_char_value.init_offs    = 0;
		attr_char_value.max_len      = 20;
    attr_char_value.p_value      = NULL;

    return sd_ble_gatts_characteristic_add(p_lbs->service_handle,
                                               &char_md,
                                               &attr_char_value,
                                               &p_lbs->mpu_char_handles);
}
uint32_t ble_mpus_init(ble_mpus_t * p_mpus, const ble_mpus_init_t * p_mpus_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure.
    p_mpus->conn_handle       = BLE_CONN_HANDLE_INVALID;
    //p_mpus->led_write_handler = p_mpus_init->led_write_handler;

    // Add service.
    ble_uuid128_t base_uuid = {LBS_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_mpus->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_mpus->uuid_type;
    ble_uuid.uuid = MAX_UUID_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_mpus->service_handle);
    VERIFY_SUCCESS(err_code);

    // Add characteristics.
    err_code = mpu_char_add(p_mpus, p_mpus_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}
/**********************************************************************************************
 * 描  述 : 使用notification将传感器数值发送给主机
 * 参  数 : p_mpus:指向服务结构体
 *        : temp[i]:MAX30102传感器检测的值
 *        : count:读取值的个数
 *        : flag:1 PPG,0 ECG
 * 返回值 : 成功返回NRF_SUCCESS，否则返回错误代码
 ***********************************************************************************************/ 

uint16_t ble_max30102_on_data_read(ble_mpus_t * p_mpus, uint8_t *temp,uint8_t count, bool flag, uint8_t *sensor_val)
{
    ble_gatts_hvx_params_t params;
    uint16_t len=20;
	  uint32_t err_code;
		uint8_t  i;  //12Byte PPG + 8Byte ECG
	  bool f1,f2;  
	  uint32_t result1=0,result2=0;
	 //数据包拼装
    if(flag==1)    //PPG count*3Byte
		{
			for(i=0;i<count*3;i++)  sensor_val[i] = *(temp+i);						
			f1=1;
		}
		else       //ECG count*2Byte
		{
		  for(i=0;i<count*2;i++)  sensor_val[12+i] = *(temp+i);
			if (f1==1)	f2=1;
		}
		
		if(f2==1)
		{  			  
//				for(int j=0;j<4;j++)      //ECG、PPG数据合成  检测数据发送正确      
//			 {							
//					result1=(long)((long)((long)sensor_val[3*j+0]&0x03)<<16) | (long)sensor_val[3*j+1]<<8 | (long)sensor_val[3*j+2];  
//					SEGGER_RTT_printf(0,"%d\r\n",result1);								
//					result2=(long)((long)((long)sensor_val[12+2*j+0]&0x03)<<8) | (long)sensor_val[12+2*j+1]; 						
//					SEGGER_RTT_printf(0,"%d\r\n",result2);
//			 }
			
//			SEGGER_RTT_printf(0,"send!\r\n");    //检测包数是否与采样率一致
			if (p_mpus->conn_handle != BLE_CONN_HANDLE_INVALID)
			{
				memset(&params, 0, sizeof(params));
				params.type = BLE_GATT_HVX_NOTIFICATION;
				params.handle = p_mpus->mpu_char_handles.value_handle;
				params.p_data = sensor_val;
				params.p_len = &len;
			
				return sd_ble_gatts_hvx(p_mpus->conn_handle, &params);
			}
			else{
				err_code = NRF_ERROR_INVALID_STATE;
			}
			f1=0;
			f2=0;
		}
			return err_code;	
}

/********************************************END FILE*******************************************/

