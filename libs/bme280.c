/** @file bme280.c
 *
 * @author Wassim FILALI
 *
 * @compiler IAR STM8
 *
 *
 * $Date: 29.10.2016 - creation
 * $Revision: 1 
 *
*/

//---------------------------------------------------------------------
//------------------------------ Includes ----------------------------- 
#include "deviceType.h"
#include "i2c_stm8x.h"

#include "clock_led.h"
#include "uart.h"

//to reconstruct the tx frame
#include "rf_protocol.h"
//---------------------------------------------------------------------
//----------------------- Registers Definition ------------------------

#define CTMS_OSRS_T_Skip     0x00
#define CTMS_OSRS_T_x1       0x20
#define CTMS_OSRS_T_x2       0x40
#define CTMS_OSRS_P_Skip     0x00
#define CTMS_OSRS_P_x1       0x04
#define CTMS_OSRS_P_x2       0x08
#define CTMS_MODE_Sleep      0x00
#define CTMS_MODE_Forced     0x01

#define CTHM_OSRS_H_Skip     0x00
#define CTHM_OSRS_H_x1       0x01
#define CTHM_OSRS_H_x2       0x02


//--------------------------------------------------------------------
//reads one single register after writing its address
//
BYTE bme280_read_reg(BYTE address)
{
    BYTE result;
    I2C_Write(0x76, &address,1);
    delay_100us();
    I2C_Read(0x76, &result,1); 
    delay_100us();
    
    return result;
}
//--------------------------------------------------------------------
//reads many registers in one pass after writing the start address only
//
void bme280_read_registers(BYTE Start, BYTE Number,BYTE *data)
{
    I2C_Write(0x76, &Start,1);//start address
    delay_100us();
    I2C_Read(0x76, data,Number); 
    delay_100us();//wait to complete before writing into unallocated variable
    //i�c repeat 3rd should be worked around here
  
}
//--------------------------------------------------------------------
//calls the reading function then prints all the read bytes
//
void bme280_print_registers(BYTE Start, BYTE Number)
{
  BYTE data[16];
  bme280_read_registers(Start,Number,data);
  printf("Reg ");
  UARTPrintfHex(Start);
  printf(" : ");
  UARTPrintfHexTable(data,Number);
  printf("\n");
}

void bme280_check_id()
{
    BYTE Id = bme280_read_reg(0xD0);//id : 0xD0
    printf("BME280 id = ");
    UARTPrintfHex(Id);
    printf("\n");
	if(Id != 0x60)
	{
		printf("Error - not expected Id from reg 0xD0 on i2C @ 0x76\n");
	}
}


//previous humidity only is interesting because the Temperature and Pressures are on the 
//same register that needs to be triggered for the measure
BYTE bme280_prev_hum = 0;

//--------------------------------------------------------------------
//stats the process of getting one measure
// Press, Temp, Hum
//1 : measure will be taken
//0 : measure will be skipped, result will read 0x80 00 ...
void bme280_force_OneMeasure(BYTE Press,BYTE Temp,BYTE Hum)
{
    BYTE data[4];

    if(Hum != bme280_prev_hum)
    {
      //Control Humidity Register
      data[0] = 0xF2;//Register address ctrl_hum 0xF2
      data[1] = CTHM_OSRS_H_Skip;
      if(Hum)
      {
        data[1] = CTHM_OSRS_H_x1;
      }
      I2C_Write(0x76, data,2);
      delay_100us();
      bme280_prev_hum = Hum;
    }

    //Control Temperature Pressure Register
    data[0] = 0xF4;//Register address ctrl_meas 0xF4
    data[1] = CTMS_MODE_Forced;
    if(Temp)
    {
      data[1] |= CTMS_OSRS_T_x1;
    }
    if(Press)
    {
      data[1] |= CTMS_OSRS_P_x1;
    }
    I2C_Write(0x76, data,2);
    delay_100us();
    
}
//--------------------------------------------------------------------
//polls the status registers to check when the read is done
//
BYTE bme280_wait_measures()
{
    BYTE status;
    BYTE count = 0;
   do
   {
     status = bme280_read_reg(0xF3);//status : 0xF3
     count++;
   }while(((status & 0x08) != 0) && (count<20));
    
    //printf("Measure done ");    UARTPrintf_uint(count);    printf(" poll\n");
	return count;
}

//--------------------------------------------------------------------
void bme280_print_status()
{
    BYTE status = bme280_read_reg(0xF2);//status : 0xF3
    printf("BME280 ctrl_hum = ");
    UARTPrintfHex(status);
    printf("\n");

    status = bme280_read_reg(0xF3);//status : 0xF3
    printf("BME280 status = ");
    UARTPrintfHex(status);
    printf("\n");

    status = bme280_read_reg(0xF4);//ctrl_meas
    printf("BME280 ctrl_meas = ");
    UARTPrintfHex(status);
    printf("\n");

    status = bme280_read_reg(0xF5);//Filter
    printf("BME280 config = ");
    UARTPrintfHex(status);
    printf("\n");
}

//--------------------------------------------------------------------
void bme280_print_CalibData()
{
    printf("BME280 calib data:\n");
    bme280_print_registers(0x88,10);
    bme280_print_registers(0x92,10);
    bme280_print_registers(0x9C,6);
    bme280_print_registers(0xE1,8);
}

//--------------------------------------------------------------------
void bme280_print_measures()
{
	//0xF7 press_msb
	//0xF8 press_lsb
	//0xF9 press_xlsb [7:4]
	//0xFA temp_msb
	//0xFB temp_lsb
	//0xFC temp_xlsb [7:4]
	//0xFD hum_msb
	//0xFE hum_lsb
	bme280_print_registers(0xF7, 8);
}
//--------------------------------------------------------------------
//get the measures from the sensor and format them for tx
//make sure the tx_data is a pre allocated 10 bytes buffer
void bme280_get_tx_payload_8B(BYTE *payload)
{
	bme280_read_registers(0xF7, 8, payload);
}

//--------------------------------------------------------------------

//Rx 11 Bytes
void bme280_rx_measures(BYTE src_NodeId,BYTE *rxPayload,BYTE rx_PayloadSize)
{
	if(rx_PayloadSize != 8)
	{
		printf("Pid:BME280;Error:dataSize not 8 but ");
		printf_uint(rx_PayloadSize);
		printf_eol();
		return;
	}
    printf("NodeId:");
    printf_uint(src_NodeId);// Byte 1 is Node Id
    printf(";BME280: ");
    printf_tab(rxPayload,rx_PayloadSize);//do not print pId nor NodeId nor CRC
    printf_eol();
}

