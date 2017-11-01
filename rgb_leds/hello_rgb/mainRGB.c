/*
	mainRGB.c for 
		IoT_Frameworks
			\rg_leds
				\hello_rgb

	started	    27.02.2017
	
*/

//------------------Target Board--------------
// Fixed Node v2
// Port B Pin 5 => Test Led is
// SPI          => nRF
// UART         
// Port A Pin 3 => RGB WS2812B

#include "clock_led.h"

#include "uart.h"

#include "WS2812B.h"

int main( void )
{
    BYTE AliveActiveCounter = 0;

    InitialiseSystemClock();

    Initialise_TestLed_GPIO_B5();
    Test_Led_Off();

    rgb_PIO_Init();
    rgb_SwitchOff_Range(0,1);//(First led, one led)
    
    uart_init();

    printf_eol();
    printf_ln("__________________________________________________");
    printf_ln("rgb_leds\\hello_rgb\\");

    printf_ln("Color Flashed R:40, G:200, B:160");
    while (1)
    {
      Test_Led_On();
      delay_ms(100);
      Test_Led_Off();

      rgb_TestColors();

      printf("Cycle:");
      printf_uint(AliveActiveCounter);
      printf_eol();
      
    }
}
