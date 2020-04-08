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
** Created date:		2019.4.17
** Version:			    ver_100Hz
** Descriptions:		Max30102原始数据读取实验、FS=100Hz
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

#include "SEGGER_RTT.h"


uint8_t temp[6];
uint32_t aun_ir_buffer; //IR LED sensor data
uint32_t aun_red_buffer;    //Red LED sensor data

void timer_handler(nrf_timer_event_t event_type, void* p_context)
{
    static uint32_t i;
	  
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
            //Do nothing.
            break;
    }    
}
void max30102_read_event_init(void)
{
    ret_code_t err_code;
	  nrf_drv_timer_t m_timer= NRF_DRV_TIMER_INSTANCE(1);
	  err_code = nrf_drv_timer_init(&m_timer,NULL,timer_handler);
		APP_ERROR_CHECK(err_code);
	
	  uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer,10);  //100Hz
	  nrf_drv_timer_extended_compare(&m_timer,
	                                  NRF_TIMER_CC_CHANNEL0,
	                                  ticks,
	                                  NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
	                                  true);
	  nrf_drv_timer_enable(&m_timer);
}

/**********************************************************************************************
 * 描  述 : main函数
 * 入  参 : 无
 * 返回值 : 无
 ***********************************************************************************************/ 
int main(void)
{
	
	  nrf_gpio_cfg_output(LED_1);//配置管脚P0.17为输出，驱动指示灯D1
    nrf_gpio_pin_set(LED_1);   //设置指示灯D1初始状态为熄灭
		twi_master_init();
	  SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
	
	  nrf_delay_ms(2000);
	  while(max30102_init(0x57) == false)
		{
				nrf_delay_ms(500);
		}
//		SEGGER_RTT_WriteString(0,"max30102 init ok\r\n");
			max30102_read_event_init();
		
    while (true)
    {
				__WFI();                                                                                                                                                       
    }
}
/********************************************END FILE*******************************************/
