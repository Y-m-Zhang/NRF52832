#include "max30102.h"
#include <stdbool.h>
#include <stdint.h>
#include "twi_master.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"


static uint8_t       m_device_address;          // !< Device address in bits [7:1]

bool max30102_register_write(uint8_t register_address, uint8_t value)
{
    uint8_t w2_data[2];

    w2_data[0] = register_address;
    w2_data[1] = value;
    return twi_master_transfer(m_device_address, w2_data, 2, TWI_ISSUE_STOP);
}
bool max30102_register_read(uint8_t register_address, uint8_t * destination, uint8_t number_of_bytes)
{
    bool transfer_succeeded;
    transfer_succeeded  = twi_master_transfer(m_device_address, &register_address, 1, TWI_DONT_ISSUE_STOP);
    transfer_succeeded &= twi_master_transfer(m_device_address|TWI_READ_BIT, destination, number_of_bytes, TWI_ISSUE_STOP);
    return transfer_succeeded;
}


bool max30102_init(uint8_t device_address)
{   
 
  bool transfer_succeeded = true;			
	NRF_GPIO->PIN_CNF[MAX30102_INT] =      \
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
      | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);	
	
  m_device_address = (uint8_t)(device_address << 1);

  //ÉèÖÃ
  max30102_register_write(REG_MODE_CONFIG,0x40);  //reset
	max30102_register_write(REG_MODE_CONFIG,0x40);  //reset
	
	max30102_register_write(REG_INTR_ENABLE_1,0xc0);	// INTR setting
	max30102_register_write(REG_INTR_ENABLE_2,0x00);
	max30102_register_write(REG_FIFO_WR_PTR,0x00);  	//FIFO_WR_PTR[4:0]
	max30102_register_write(REG_OVF_COUNTER,0x00);  	//OVF_COUNTER[4:0]
	max30102_register_write(REG_FIFO_RD_PTR,0x00);  	//FIFO_RD_PTR[4:0]
	max30102_register_write(REG_FIFO_CONFIG,0x0f);  	//sample avg = 1, fifo rollover=false, fifo almost full = 17
	max30102_register_write(REG_MODE_CONFIG,0x02);  	//0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
//	max30102_register_write(REG_MODE_CONFIG,0x03);  	//0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
//	max30102_register_write(REG_SPO2_CONFIG,0x27);  	// SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)  
	max30102_register_write(REG_SPO2_CONFIG,0x2b);  	// SPO2_ADC range = 4096nA, SPO2 sample rate (200 Hz), LED pulseWidth (400uS) 
//	max30102_register_write(REG_LED1_PA,0x24);   	//Choose value for ~ 7mA for LED1
//	max30102_register_write(REG_LED2_PA,0x24);   	// Choose value for ~ 7mA for LED2
	max30102_register_write(REG_LED1_PA,0x30);   	//Choose value for ~ 7mA for LED1
	max30102_register_write(REG_LED2_PA,0x30);   	// Choose value for ~ 7mA for LED2
	max30102_register_write(REG_PILOT_PA,0x7f);   	// Choose value for ~ 25mA for Pilot LED
					
  return transfer_succeeded;
}







