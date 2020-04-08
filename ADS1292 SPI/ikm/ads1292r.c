#include "ads1292r.h"
#include "nrf_drv_spi.h"
#include "nrf_delay.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "app_uart.h"

#define SPI_CS_PIN   22  
#define SPI_BUFSIZE  10

#define SPI_INSTANCE  0 /**< SPI instance index. */

uint8_t   SPI_Tx_Buf[SPI_BUFSIZE];
uint8_t   SPI_Rx_Buf[SPI_BUFSIZE];
volatile  uint8_t   SPIReadLength, SPIWriteLength;
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

char ads1292r_data_buff[9];


void spi_event_handler(nrf_drv_spi_evt_t const * p_event)
{
    spi_xfer_done = true;
}

void SPI_Init(void)
{		
	  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG(SPI_INSTANCE);
		spi_config.mode = NRF_DRV_SPI_MODE_1;
	  spi_config.frequency = NRF_DRV_SPI_FREQ_250K;
    spi_config.ss_pin = SPI_CS_PIN; 
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler));
	
  	nrf_delay_ms(200);
	
		ADS1292R_START_L;
		ADS1292R_PWDN_H;
	
}
 
/**ADS1292R上电复位 **/
void ADS1292R_Init(void)
{
	uint8_t device_id ,test;

	ADS1292R_START_H;      
	ADS1292R_PWDN_L;//进入掉电模式
	nrf_delay_ms(10);
	ADS1292R_PWDN_H;//退出掉电模式
	nrf_delay_ms(10);//等待稳定
	ADS1292R_PWDN_L;//发出复位脉冲
	nrf_delay_ms(10);
	ADS1292R_PWDN_H;
	nrf_delay_ms(10);//等待稳定，可以开始使用ADS1292R
	ADS1292R_START_L;
	

	ADS1292R_CMD(ADS1292R_SDATAC);//发送停止连续读取数据命令

	nrf_delay_ms(1);
	while(device_id!=0x73)       //识别芯片型号，1292r为0x73
	{
		device_id = ADS1292R_ReadReg(ADS1292R_ID);
		SEGGER_RTT_printf(0,"can't find device!\r\n");
		nrf_delay_ms(10);
	}
		
//	ADS1292R_WriteReg(ADS1292R_CONFIG2,0XE0);	//使用内部参考电压
//	nrf_delay_ms(10);//等待内部参考电压稳定
//	ADS1292R_WriteReg(ADS1292R_CONFIG1,0X02);	//设置转换速率为500SPS
//	ADS1292R_WriteReg(ADS1292R_LOFF,0XF0);
//	ADS1292R_WriteReg(ADS1292R_CH1SET,0X00);
//	ADS1292R_WriteReg(ADS1292R_CH2SET,0x00);
//	ADS1292R_WriteReg(ADS1292R_RLD_SENS,0x30);
//	//	ADS1292R_WriteReg(ADS1292R_RLD_SENS,0x3C);	//使用通道2提取共模电压
//	ADS1292R_WriteReg(ADS1292R_LOFF_SENS,0x3F);
//	//  ADS1292R_WriteReg(LOFF_STAT,0X00);
//	ADS1292R_WriteReg(ADS1292R_RESP1,0xDE);
//	ADS1292R_WriteReg(ADS1292R_RESP2,0x07);
//	ADS1292R_WriteReg(ADS1292R_GPIO,0x0C);
	
	
	ADS1292R_WriteReg(ADS1292R_CONFIG2,0XA0);	//使用内部参考电压
	nrf_delay_ms(10);//等待内部参考电压稳定
	ADS1292R_WriteReg(ADS1292R_CONFIG1,0X00);	//设置转换速率为125SPS		
	ADS1292R_WriteReg(ADS1292R_LOFF,0X10);
	ADS1292R_WriteReg(ADS1292R_CH1SET,0X40);
	ADS1292R_WriteReg(ADS1292R_CH2SET,0x60);
	ADS1292R_WriteReg(ADS1292R_RLD_SENS,0x2C);
	ADS1292R_WriteReg(ADS1292R_LOFF_SENS,0x00);  //导联检测 关闭
	ADS1292R_WriteReg(ADS1292R_RESP1,0xF2);
	ADS1292R_WriteReg(ADS1292R_RESP2,0x03);
	
	nrf_delay_ms(1);
	ADS1292R_CMD(ADS1292R_RDATAC);
	ADS1292R_START_H;//启动转换

}
//void ADS1292R_Work(void)
//{
//	ADS1292R_CMD(ADS1292R_RDATAC);//回到连续读取数据模式，检测噪声数据
//	ADS1292R_START_H;//启动转换
//}
void ADS1292R_Halt(void)
{
	ADS1292R_START_H;//启动转换
	ADS1292R_CMD(ADS1292R_SDATAC);//发送停止连续读取数据命令
}
//对ADS1292R写入指令
void ADS1292R_CMD(uint8_t cmd)
{
		spi_xfer_done = false;
		nrf_delay_ms(1);
	  
    SPIWriteLength = 1;
    SPIReadLength = 0;
    SPI_Tx_Buf[0] = cmd;  
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, SPI_Tx_Buf, SPIWriteLength, SPI_Rx_Buf, SPIReadLength));
    while(spi_xfer_done == false);	
}

uint8_t ADS1292R_ReadReg(int reg)
{	
		spi_xfer_done = false;
    SPIWriteLength = 2;
    SPIReadLength = 1;
    SPI_Tx_Buf[0] =(ADS1292R_RREG | reg);
	  SPI_Tx_Buf[1] = 0x00;         //读出的数量-1
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, SPI_Tx_Buf, SPIWriteLength, SPI_Rx_Buf, SPIReadLength));
	  while(spi_xfer_done == false);

		return  SPI_Rx_Buf[0];
}


void ADS1292R_WriteReg(uint8_t WriteAddr, uint8_t Data)
{
	  switch (WriteAddr)
  {
    case 1:
			Data = Data & 0x87;
	    break;
    case 2:
			Data = Data & 0xFB;
	    Data |= 0x80;		
	    break;
    case 3:
	    Data = Data & 0xFD;
	    Data |= 0x10;
	    break;
    case 7:
	    Data = Data & 0x3F;
	    break;
    case 8:
			Data = Data & 0x5F;
	    break;
    case 9:
	    Data |= 0x02;
	    break;
    case 10:
	    Data = Data & 0x87;
	    Data |= 0x01;
	    break;
    case 11:
	    Data = Data & 0x0F;
	    break;
    default:
	    break;		
  }
	  uint8_t dataToSend = ADS1292R_WREG | WriteAddr ;

    spi_xfer_done = false;
    SPIWriteLength = 3;
    SPIReadLength = 0;
    SPI_Tx_Buf[0] = dataToSend;
	  SPI_Tx_Buf[1] = 0x00;         //写入的数量-1
    SPI_Tx_Buf[2] = (Data);
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, SPI_Tx_Buf, SPIWriteLength, SPI_Rx_Buf, SPIReadLength));
	  while(spi_xfer_done == false);

}

/*读取72位的数据1100+LOFF_STAT[4:0]+GPIO[1:0]+13个0+2CHx24位，共9字节*/
char* ADS1292R_ReadData(void)
{
	spi_xfer_done = false;
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, SPI_Tx_Buf, 0, SPI_Rx_Buf, 9));
	while(spi_xfer_done == false);

	for(int i=0;i<9;i++)
	{
		ads1292r_data_buff[i]=SPI_Rx_Buf[i];
	}
	return ads1292r_data_buff;
}





