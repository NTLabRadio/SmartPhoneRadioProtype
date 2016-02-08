#include <stdio.h>
#include "stm32f1xx_hal.h"


#pragma import(__use_no_semihosting_swi)

#define ECHO_FGETC

volatile int ITM_RxBuffer=0x5AA55AA5; /* Buffer to transmit data towards debug system. */

struct __FILE { int handle; };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) 
{
	return (ITM_SendChar((uint32_t)ch));
}	
