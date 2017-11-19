#include <iostm8l151f3.h>
#include <intrinsics.h>

//should be before other files that use it such as clock_led.h
#include "deviceType.h"
#include "nRF_Configuration.h"

#include "nRF_SPI.h"
//for nRF_SetMode_TX()
#include "nRF.h"

#include "nRF_Tx.h"

#include "uart.h"
#include "clock_led.h"


#define EEPROM_Offset 0x1000
#define NODE_HW_CONFIG	EEPROM_Offset+0x10
#define NODE_FUNCTIONAL_CONFIG	EEPROM_Offset+0x20

#define NODE_ID       (char *) EEPROM_Offset;
BYTE NodeId;

//to format the tx data
#include "rf_protocol.h"
#include "cmdutils.h"

BYTE tx_data[RF_MAX_DATASIZE];

//---------------------- Active Halt Mode :
// - CPU and Peripheral clocks stopped, RTC running
// - wakeup from RTC, or external/Reset


//Magnet B0 is Top
//Magnet D0 is side
//------------------------------ Node Config ---------------------------------
#define NODE_MAGNET_B_SET               *(char*)(NODE_HW_CONFIG+0x00)
#define NODE_MAGNET_B_INTERRUPT			*(char*)(NODE_HW_CONFIG+0x01)
#define NODE_MAGNET_D_SET               *(char*)(NODE_HW_CONFIG+0x02)
#define NODE_MAGNET_D_INTERRUPT         *(char*)(NODE_HW_CONFIG+0x03)
#define NODE_I2C_SET                    *(char*)(NODE_HW_CONFIG+0x04)
#define NODE_MAX44009_SET               *(char*)(NODE_HW_CONFIG+0x05)

#define SLEEP_PERIOD_SEC				*(char*)(NODE_FUNCTIONAL_CONFIG+0x00)

void rf_alive_bcast()
{
    
    tx_data[rfi_size] = rfi_broadcast_header_size;
    tx_data[rfi_ctr] = rf_ctr_Broadcast | 2;//time to live is 2
    tx_data[rfi_pid] = rf_pid_alive;
    tx_data[rfi_src] = NodeId;
    crc_set(tx_data);
   
	nRF_Transmit_Wait_Down(tx_data,rfi_broadcast_header_size+crc_size);
}

void RfSwitch(unsigned char state)
{
	/*
      unsigned char Tx_Data[4];
      Tx_Data[0]=0xC5;
      Tx_Data[1]=NodeId;
      Tx_Data[2]=state;
      Tx_Data[3]= Tx_Data[0] ^ NodeId ^ state;
      nRF_Transmit_Wait_Down(Tx_Data,4);
	  */
}


void LogMagnets()
{
      delay_100us();
      unsigned char Magnet_B0,Magnet_D0;
      Magnet_B0 = PB_IDR_IDR0;
      Magnet_D0 = PD_IDR_IDR0;
      //printf(" LVD0 ");
      UARTPrintf_uint(Magnet_D0);
      //printf(" ; HH B0 ");
      UARTPrintf_uint(Magnet_B0);
      printf("\n");
      delay_100us();
      delay_100us();
}

void i2c_user_Rx_Callback(BYTE *userdata,BYTE size)
{
	/*printf("I2C Transaction complete, received:\n\r");
	UARTPrintfHexTable(userdata,size);
	printf("\n\r");*/
        
}

void i2c_user_Tx_Callback(BYTE *userdata,BYTE size)
{
  /*
	printf("I2C Transaction complete, Transmitted:\n\r");
	UARTPrintfHexTable(userdata,size);
	printf("\n\r");
        */
}

void i2c_user_Error_Callback(BYTE l_sr2)
{
	if(l_sr2 & 0x01)
	{
		printf("[I2C Bus Error]\n\r");
	}
	if(l_sr2 & 0x02)
	{
		printf("[I2C Arbitration Lost]\n\r");
	}
	if(l_sr2 & 0x04)
	{
		printf("[I2C no Acknowledge]\n\r");//this is ok for the slave
	}
	if(l_sr2 & 0x08)
	{
		printf("[I2C Bus Overrun]\n\r");
	}
}

//B0 : bit 0 - pin interrupt
#pragma vector = EXTI0_vector
__interrupt void IRQHandler_Pin0(void)
{
  if(EXTI_SR1_P0F == 1)
  {
	if(NODE_MAGNET_B_SET == 1)
		RfSwitch(PB_IDR_IDR0);//B0 is Side
	else if(NODE_MAGNET_D_SET == 1)
		RfSwitch(PD_IDR_IDR0);//D0 is Top
  }
  EXTI_SR1 = 0xFF;//acknowledge all interrupts pins
}


#pragma vector = RTC_WAKEUP_vector
__interrupt void IRQHandler_RTC(void)
{
  if(RTC_ISR2_WUTF)
  {
    RTC_ISR2_WUTF = 0;
    
  }
  
}


void SMT8L_Switch_ToHSI()
{
  CLK_SWCR_SWEN = 1;                  //  Enable switching.
  CLK_SWR = 0x01;                     //  Use HSI as the clock source.
  while (CLK_SWCR_SWBSY != 0);        //  Pause while the clock switch is busy.
}


void Init_Magnet_PB0()
{
  if(NODE_MAGNET_B_SET == 1)
  {
      PB_DDR_bit.DDR0 = 0;//  0: Input
      PB_CR1_bit.C10 = 0; //  0: Floating
          if(NODE_MAGNET_B_INTERRUPT == 1)
		  {
			PB_CR2_bit.C20 = 1; // Exernal interrupt enabled
			EXTI_CR1_P0IS = 3;//Rising and Falling edges, interrupt on events - bit 0
		  }
		  else
		  {
			PB_CR2_bit.C20 = 0; // Exernal interrupt disabled
		  }
  }
    //EXTI_CR3_PBIS = 00;//Falling edge and low level - Port B
}

void Init_Magnet_PD0()
{
  if(NODE_MAGNET_B_SET == 1)
  {
    PD_DDR_bit.DDR0 = 0;//  0: Input
    PD_CR1_bit.C10 = 0; //  0: Floating
	if(NODE_MAGNET_D_INTERRUPT == 1)
	{
    	PD_CR2_bit.C20 = 1; // Exernal interrupt enabled
    	EXTI_CR1_P0IS = 3;//Rising and Falling edges, interrupt on events - bit 0
	}
	else
	{
   		PD_CR2_bit.C20 = 0; // Exernal interrupt disabled
	}
  }
}


void Initialise_Test_GPIO_A2()
{
    PA_DDR_bit.DDR2 = 1;//  1: Output
    PA_CR1_bit.C12 = 1; //  1: Push-pull
}

void GPIO_B3_High()
{
    PB_ODR_bit.ODR3 = 1;
}

void GPIO_B3_Low()
{
    PB_ODR_bit.ODR3 = 0;
}

void configure_All_PIO()
{
	//A0 - SWIM
	//PA_DDR_bit.DDR3 = 1;//output
	//PA_ODR_bit.ODR3 = 0;//Low
	//A1 - NRST
	//PA_DDR_bit.DDR3 = 1;//output
	//PA_ODR_bit.ODR3 = 0;//Low
	//A2 - UART-Tx
	PA_DDR_bit.DDR2 = 1;//output
	PA_ODR_bit.ODR2 = 0;//Low
	//A3 - unconnected
	PA_DDR_bit.DDR3 = 1;//output
	PA_ODR_bit.ODR3 = 0;//Low

	//B0 - Magnet-1 Side
	if(NODE_MAGNET_B_SET != 1)
	{
		PB_DDR_bit.DDR0 = 0;//output
		PB_ODR_bit.ODR0 = 0;//Low
	}
	if(NODE_MAX44009_SET != 1)
	{
			//B1 - Light-IRQ
		PB_DDR_bit.DDR1 = 1;//output
		PB_ODR_bit.ODR1 = 0;//Low
	}
	//B2 - unconnected
	PB_DDR_bit.DDR2 = 1;//output
	PB_ODR_bit.ODR2 = 0;//Low
        //B3 - nRF CE_Pin_LowDisable()
	PB_DDR_bit.DDR3 = 1;//output
	PB_ODR_ODR3 = 0;
        //B4 - nRF CSN High Disable
	PB_DDR_bit.DDR4 = 1;//output
	PB_ODR_ODR4 = 1;
        //B5 - nRF SPI-SCK
	PB_DDR_bit.DDR5 = 1;//output
	PB_ODR_ODR5 = 0;
        //B6 - nRF SPI-MOSI
	PB_DDR_bit.DDR6 = 1;//output
	PB_ODR_ODR6 = 1;
        //B7 - nRF SPI-MISO
	PB_DDR_bit.DDR7 = 0;//input
	//PB_ODR_ODR7 = 1;

	if(NODE_I2C_SET != 1)
	{
		//C0 - I�C SDA
		PC_DDR_bit.DDR0 = 1;//output
		PC_ODR_bit.ODR0 = 0;//Low
		//C1 - I�C SCL
		PC_DDR_bit.DDR1 = 1;//output
		PC_ODR_bit.ODR1 = 0;//Low
	}
        //C2-C3 : do not exist
	//C4 - nRF IRQ
	PC_DDR_bit.DDR4 = 0;//input
	//PC_ODR_bit.ODR4 = 0;
        //C5 - Osc 1
	PC_DDR_bit.DDR5 = 1;//output
	PC_ODR_bit.ODR5 = 0;//Low
	//C6 - Osc 2
	PC_DDR_bit.DDR6 = 1;//output
	PC_ODR_bit.ODR6 = 0;//Low

	//D0 - Magnet-2 Top
	if(NODE_MAGNET_D_SET != 1)
	{
		PD_DDR_bit.DDR0 = 1;//output
		PD_ODR_bit.ODR0 = 0;//Low
	}

}

int main( void )
{
	NodeId = *NODE_ID;

	configure_All_PIO();
	Init_Magnet_PB0();//conditionned config with flags
	Init_Magnet_PD0();//conditionned config with flags
	
	Initialise_STM8L_Clock();		//here enable the RTC clock

	//#issue cannot change after first config
	sleep(SLEEP_PERIOD_SEC);						//this is a low power halt sleep 
	Initialise_STM8L_RTC_LowPower(SLEEP_PERIOD_SEC);//configure the sleep cycle for a period of 30 sec
    
	//SYSCFG_RMPCR1_USART1TR_REMAP = 1; // Remap 01: USART1_TX on PA2 and USART1_RX on PA3
	//uart_init();//UART Disabled
	//Applies the compile time configured parameters from nRF_Configuration.h
	nRF_Config();
    nRF_SelectChannel(10);

	__enable_interrupt();
    //
    // Main loop
    //
    while (1)
    {
		__halt();

		//here we wake up from halt
		rf_alive_bcast();//using nRF_Transmit_Wait_Down()

    }
}
