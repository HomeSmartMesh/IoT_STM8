/*

	main.c for 
		IoT_Frameworks
			\light_dimmer
				\simple_monitor

	started	05.11.2016
	
*/

//Test Led is Port B Pin 5


#include "uart.h"
#include "clock_led.h"

#include "light_dimmer_krida.h"

#include <intrinsics.h>

int main( void )
{

	dimmer_init();//fast config - enables the PB4 interrupt

    BYTE AliveActiveCounter = 0;

    InitialiseSystemClock();


    Initialise_TestLed_GPIO_B5();
    Test_Led_Off();

    uart_init();
	
	printf_eol();
    printf_ln("__________________________________________________");
    printf_ln("IoT_Frameworks\\light_dimmer\\simple_monitor\\");

	dimmer_set_level(0,1800);//60us is the Sync shift + 10 to be in the positive section
	dimmer_set_level(1,2200);//60us is the Sync shift
	dimmer_set_level(2,2600);//60us is the Sync shift
	dimmer_set_level(3,3000);//60us is the Sync shift

    __enable_interrupt();
	U16_t last_count = get_int_count();
    while (1)
    {

		printf("int count: ");
		U16_t count = get_int_count();
		printf_uint(count - last_count);
		last_count = count;
		printf_eol();

		AliveActiveCounter++;//Why are you counting ?
		
		Test_Led_Off();
		delay_ms(4900);
		
		Test_Led_On();
		delay_ms(100);
    }
}
