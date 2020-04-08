/****************************************Copyright (c)****************************************************
**                                        
**                                 
**
**--------------File Info---------------------------------------------------------------------------------
** File name:			     main.c
** Last modified Date:       
** Last Version:		   
** Descriptions:		   使用的SDK版本-SDK_11.0.0
**				
**--------------------------------------------------------------------------------------------------------
** Created by:		  zym
** Created date:		2019.4.8
** Version:			    ver_无BLE
** Descriptions:		MAX30102 + AD8232数据读取 (IIC-100hz ADC-250hz)
**--------------------------------------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "twi_master.h"
#include "max30102.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_saadc.h"

#include "SEGGER_RTT.h"

#define SAMPLES_IN_BUFFER 1                                 /**< ADC采样数据缓存大小(字节数)  */
static nrf_saadc_value_t       m_buffer_pool[2][SAMPLES_IN_BUFFER];
uint8_t temp[6];
uint32_t aun_ir_buffer;     //IR LED sensor data
uint32_t aun_red_buffer;    //Red LED sensor data


void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;
        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);
				SEGGER_RTT_printf(0,"%d\r\n",p_event->data.done.p_buffer[0]);
    }
}

void saadc_init(void) 
{
    ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config =
		NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);
    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0],SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
    
    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1],SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
}

void timer_handler(nrf_timer_event_t event_type, void* p_context)
{
    static uint32_t i;
    ret_code_t err_code;
    switch(event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
							err_code = nrf_drv_saadc_sample();
							APP_ERROR_CHECK(err_code);		
			    break;
			 
        default:
            break;
    }    					
}
void saadc_sampling_event_init(void)
{
    ret_code_t err_code;
	  nrf_drv_timer_t m_timer= NRF_DRV_TIMER_INSTANCE(1);
	  err_code = nrf_drv_timer_init(&m_timer,NULL,timer_handler);
		APP_ERROR_CHECK(err_code);
	
	  uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer,4);
	  nrf_drv_timer_extended_compare(&m_timer,
	                                  NRF_TIMER_CC_CHANNEL0,
	                                  ticks,
	                                  NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
	                                  true);
	  nrf_drv_timer_enable(&m_timer);
	
}

void timer2_handler(nrf_timer_event_t event_type, void* p_context)
{
    switch(event_type)
    {
       case NRF_TIMER_EVENT_COMPARE0:
			 NRF_TIMER1->EVENTS_COMPARE[0]=0;
			 while(MAX30102_INT==1);
			 max30102_register_read(REG_FIFO_DATA,temp, 6);
			 aun_red_buffer =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];   
//			 aun_ir_buffer = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5]; 
			 SEGGER_RTT_printf(0,"%d\r\n",aun_red_buffer);
			    break;
        
        default:
            break;
    }    
}
void max30102_read_event_init(void)
{
	  ret_code_t err_code;  
		nrf_drv_timer_t m_timer1= NRF_DRV_TIMER_INSTANCE(2);
	  err_code = nrf_drv_timer_init(&m_timer1,NULL,timer2_handler);
		APP_ERROR_CHECK(err_code);
	  uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer1,10);
	  nrf_drv_timer_extended_compare(&m_timer1,
	                                  NRF_TIMER_CC_CHANNEL0,
	                                  ticks,
	                                  NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
	                                  true);																
	  nrf_drv_timer_enable(&m_timer1);
}

/**********************************************************************************************
 * 描  述 : main函数
 * 入  参 : 无
 * 返回值 : 无
 ***********************************************************************************************/ 
int main(void)
{
		twi_master_init();
	  saadc_init();
	  SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
	  nrf_delay_ms(2000);
	  while(max30102_init(0x57) == false)
		{
			  SEGGER_RTT_WriteString(0,"max30102 init failed\r\n");
				nrf_delay_ms(500);
		}
		max30102_read_event_init();
		saadc_sampling_event_init();
		
    while (true)
    {
				__WFI();
    }
}
/********************************************END FILE*******************************************/
