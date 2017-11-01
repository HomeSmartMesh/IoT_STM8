/** @file cmdutils.c

 @author Wassim FILALI  STM8 S

 @compiler IAR STM8


 $Date: 18.11.2016
 $Revision:

*/

#include "cmdutils.h"

#include "eeprom.h"

#include "uart.h"

void handle_command(BYTE *buffer,BYTE size);

BYTE strcmp (BYTE * s1, const char * s2)
{
    for(; *s1 == *s2; ++s1, ++s2)
        if(*s1 == 0)
            return 0;
    return *(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1;
}

BYTE strbegins (BYTE * s1, const char * s2)
{
    for(; *s1 == *s2; ++s1, ++s2)
        if(*s2 == 0)
            return 0;
    return (*s2 == 0)?0:1;
}

BYTE get_hex_char(BYTE c)
{
	BYTE res = 0;
	if(c <= '9')
	{
		res = c - '0';
	}
	else if(c <= 'F')
	{
		res = c - 'A' + 10;
	}
	else if(c <= 'f')
	{
		res = c - 'a' + 10;
	}
	return res;
}

BYTE get_hex(BYTE* buffer,BYTE pos)
{
	BYTE hex_val;
	pos+=2;//skip "0x"
	hex_val = get_hex_char(buffer[pos++]);
	hex_val <<= 4;
	hex_val |= get_hex_char(buffer[pos]);
	return hex_val;
}

BYTE line_length(BYTE*rxData,BYTE max_size)
{
	for(BYTE i=0;i<max_size;i++)
	{
		if(*rxData++ == UART_EOL_C)
		{
			return i+1;//+1 is to keep the '\n'
		}
	}
	return max_size;
}

//	NodeId
//	Command Lines Script 0x02
void run_eeprom_script()
{
	//as line 1 has the node id, start from the second
	BYTE *pCmd = (BYTE*)EEPROM_Offset + EEPROM_Line;
	if(strbegins(pCmd,"Command Lines Script") == 0)
	{
		BYTE NbCommands = get_hex(pCmd,21);
		printf_ln("=>Running EEProm Script");
		printf_uint(NbCommands);
		printf_ln(" Commands :");
		//Jump to the next line
		pCmd += EEPROM_Line;
		for(BYTE i=0;i<NbCommands;i++)
		{
			handle_command(pCmd,EEPROM_Line);
			pCmd += EEPROM_Line;
		}
	}
	else
	{
		printf_ln("No EEProm Script found");
	}
	
}

uint16_t crc_Fletcher16( uint8_t const *data, uint8_t count )
{
	uint16_t sum1 = 0;
	uint16_t sum2 = 0;
	int index;

	for( index = 0; index < count; ++index )
	{
		sum1 = (sum1 + data[index]) % 255;
		sum2 = (sum2 + sum1) % 255;
	}

	return (sum2 << 8) | sum1;
}

// size(size+data) : data : crc
void crc_set(uint8_t *data)
{
	uint8_t size = data[0];
	uint16_t crc = crc_Fletcher16(data,size);//check the data without excluding the crc
	data[size]   = (crc >> 8);
	data[size+1] = (crc & 0xFF);
}

// size(size+data) : data : crc
BYTE crc_check(uint8_t const *data)
{
	BYTE result = 1;
	uint8_t size = data[0];
	uint16_t crc = crc_Fletcher16(data,size);//check the data without excluding the crc
	if( (data[size] != (crc >> 8) ) || (data[size+1] != (crc & 0xFF) ) )
	{
		result = 0;
	}
	return result;
}
