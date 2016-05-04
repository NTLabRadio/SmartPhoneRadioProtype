
#include <string.h>

#ifdef STM32F071xB
#include "stm32f0xx_hal.h"
#endif
	 
#ifdef STM32F103xE	 
#include "stm32f1xx_hal.h"
#endif

#define SIZE_OF_CODED_FRAME		(196)
#define SIZE_OF_UNCODED_FRAME	(96)

//–азмер состо€ни€ FSM (автомата конечного состо€ни€) кодера, бит
#define SIZE_OF_TRELLIS_STATE	(4)
//–азмер выходного созвезди€
#define SIZE_OF_TRELLIS_OUTPUT_CONSTELLATION 		(SIZE_OF_TRELLIS_STATE*SIZE_OF_TRELLIS_STATE)

void trellisEnc1_2(int16_t * message, int16_t * out);
int16_t trellisDec1_2( int16_t *in, int16_t *out);

void TestTrellisCoder();
