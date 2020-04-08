/****************************************Copyright (c)****************************************************
**                                        
**                                 
**
**--------------File Info---------------------------------------------------------------------------------
** File name:			     main.c
** Last modified Date: 2016.3.27         
** Last Version:		   1.1
** Descriptions:		   ʹ�õ�SDK�汾-SDK_11.0.0
**				
**--------------------------------------------------------------------------------------------------------
** Created by:			
** Created date:		2015-7-1
** Version:			    1.0
** Descriptions:		LIS3DH��дʵ��
**--------------------------------------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "ads1292r.h"
#include "nrf_drv_spi.h"
#include "SEGGER_RTT.h"


//��ʵ�鿪�����LIS3DH֮��ʹ�õ���SPI����ͨѶ
/* ��������LIS3DHģ��ʹ���ռ�õ�nRF52832�ܽ���Դ
P0.06��UART_TXD   �����ڷ���
P0.08��UART_RXD   �����ڽ���
P0.07��UART_CTS   : δʹ������
P0.05��UART_RTS   : δʹ������


*/

#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 1                           /**< UART RX buffer size. */


void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}
/**********************************************************************************************
 * ��  �� : ���ڳ�ʼ����������=115200bps
 * ��  �� : ��
 * ����ֵ : ��
 *********************************************************************************************/ 
void uart_init(void)
{
	  uint32_t err_code;
    const app_uart_comm_params_t comm_params =
    {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_DISABLED,
          false,
          UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOW,
                         err_code);

    APP_ERROR_CHECK(err_code);
}

void data_deal(uint8_t data_buf[9])
{
    int i,j=0;
		unsigned long uecgtemp = 0;
		signed long secgtemp=0;
		volatile signed long s32DaqVals;
		uint8_t DataPacketHeader[16];

//    for(i=0;i<6;i+=3)                  // data outputs is (24 status bits + 24 bits Respiration data +  24 bits ECG data) 
//    {
//        uecgtemp = (unsigned long) (((unsigned long)data_buf[i+3] << 16) | ( (unsigned long) data_buf[i+4] << 8) |  (unsigned long) data_buf[i+5]);
//        uecgtemp = (unsigned long) (uecgtemp << 8);
//        secgtemp = (signed long) (uecgtemp);
//        secgtemp = (signed long) (secgtemp >> 8);
//					
//        s32DaqVals[j]=secgtemp;
//				SEGGER_RTT_printf(0,"%d\r\n",s32DaqVals[j]);
//				j++;
//    }

        uecgtemp = (unsigned long) (((unsigned long)data_buf[6] << 16) | ( (unsigned long) data_buf[7] << 8) |  (unsigned long) data_buf[8]);
        uecgtemp = (unsigned long) (uecgtemp << 8);
        secgtemp = (signed long) (uecgtemp);
        secgtemp = (signed long) (secgtemp >> 8);	
        s32DaqVals=secgtemp;
				
				SEGGER_RTT_printf(0,"%d\r\n",s32DaqVals);

   //���ݰ�15 ��ͷ��β
    DataPacketHeader[0] = 0x0A;   // Packet header1 :0x0A  
    DataPacketHeader[1] = 0xFA;    // Packet header2 :0xFA
    DataPacketHeader[2] = 0;
    DataPacketHeader[3] = 0;
    DataPacketHeader[4] = 0x02;      // packet type: 0x02 -data 0x01 -commmand

    DataPacketHeader[5] = s32DaqVals;            // 4 bytes ECG data
    DataPacketHeader[6] = s32DaqVals>>8;
    DataPacketHeader[7] = s32DaqVals>>16;
    DataPacketHeader[8] = s32DaqVals>>24; 
    
//    DataPacketHeader[9] = s32DaqVals[0];            // 4 bytes Respiration data
//    DataPacketHeader[10] = s32DaqVals[0]>>8;
//    DataPacketHeader[11] = s32DaqVals[0]>>16;
//    DataPacketHeader[12] = s32DaqVals[0]>>24; 

    DataPacketHeader[13] = 0x00;   // Packet footer1:0x00
    DataPacketHeader[14] = 0x0B ;   // Packet footer2:0x0B

//    for(i=0; i<15; i++) 
//    {
//      Serial.write(DataPacketHeader[i]);     // transmit the data over USB
//     } 
}

/**********************************************************************************************
 * ��  �� : main����
 * ��  �� : ��
 * ����ֵ : ��
 *********************************************************************************************/ 
int main(void)
{
		char *data_ptr;
		uint8_t data_buf[9];
//	  nrf_gpio_cfg_output(LED_1); //����P0.17Ϊ���������ָʾ��D1
//	  nrf_gpio_cfg_output(LED_2);
//	  nrf_gpio_pin_set(LED_1);
//	  nrf_gpio_pin_set(LED_2);
//	  nrf_gpio_range_cfg_input(BUTTON_START,BUTTON_STOP,NRF_GPIO_PIN_PULLUP);//����P0.17~P0.20Ϊ����
//    uart_init();
//		SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
//	  nrf_delay_ms(100);
		SPI_Init();
		
		nrf_gpio_cfg_input(ADS1292R_ADC_RDY_PIN,GPIO_PIN_CNF_PULL_Pullup); 
	  nrf_gpio_cfg_output(ADS1292R_START_PIN);
		nrf_gpio_cfg_output(ADS1292R_PWDN_PIN);
		
	  ADS1292R_Init();
	
    while (true)
    {
			if(nrf_gpio_pin_read(ADS1292R_ADC_RDY_PIN)==0)
			{
			    data_ptr=ADS1292R_ReadData();
					for(int i = 0; i < 9; i++)
					{
							data_buf[i] = *(data_ptr + i);  // store the result data in array
//						  SEGGER_RTT_printf(0,"%d\r\n",data_buf[i]);
					}
				  data_deal(data_buf);
			}				
    }
}

/********************************************END FILE*******************************************/
