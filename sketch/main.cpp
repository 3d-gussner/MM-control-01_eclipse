//main.cpp

#include "main.h"
#include "../core/Arduino.h"
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include "shr16.h"
#include "adc.h"
#include "uart.h"
#include "spi.h"
#include "tmc2130.h"
#include "abtn3.h"
#include "mmctl.h"
#include "motion.h"
#include "Buttons.h"


int8_t sys_state = 0;
uint8_t sys_signals = 0;
int _loop = 0;

 
extern "C" {
void process_commands(FILE* inout);


//int buttonClicked();
}

//initialization after reset
void setup()
{
	shr16_init(); // shift register
	shr16_set_led(0x3ff); // set all leds on
	uart1_init(); //uart1

#if (UART_STD == 1)
	stdin = uart1io; // stdin = uart1
	stdout = uart1io; // stdout = uart1
#endif //(UART_STD == 1)

	spi_init();
	tmc2130_init(); // trinamic
	adc_init(); // ADC

	delay(50);

  shr16_set_ena(7);
  shr16_set_led(0x000);
	
	home();
	
	Serial.begin(115200);

	
}

//main loop
void loop()
{
	process_commands(uart1io);
 



	if (!isPrinting)
	{
		int _button = buttonClicked();
		if (_button != 0) 
		{ 
			delay(1000); 
			int _button = buttonClicked();
			if (_button == 1)
			{
				if (active_extruder < 4)
				{
					select_extruder(active_extruder+1);
				}
			}
			if (_button == 4)
			{
				if (active_extruder > 0)
				{
					select_extruder(active_extruder-1);
				}
			}
		}
		
		
	}
	 
}


extern "C" {

void process_commands(FILE* inout)
{
	static char line[32];
	static int count = 0;
	int c = -1;
	if (count < 32)
	{
		if ((c = getc(inout)) >= 0)
		{
			if (c == '\r') c = 0;
			if (c == '\n') c = 0;
			line[count++] = c;
		}
	}
	else
	{
		count = 0;
		//overflow
	}
	int value = 0;

	if ((count > 0) && (c == 0))
	{ 
		//line received
		printf_P(PSTR("line received: '%s' %d\n"), line, count);
		count = 0;
		bool retOK = false;


		if (sscanf_P(line, PSTR("T%d"), &value) > 0)
		{ 
			//T-code scanned
			if ((value >= 0) && (value < EXTRUDERS))
			{
				retOK = switch_extruder(value);
				
				delay(200);

				if (retOK)
				{
					fprintf_P(inout, PSTR("ok\n"));
					load_filament_inPrinter();
				}
				else
				{
					fprintf_P(inout, PSTR("ok\n"));
				}
				
			}
		}


		

	}
	else
	{ //nothing received
	}
}



}
