#include <cr_section_macros.h>

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

// Pin Name: P1_1
#define PORT    0
#define BIT     8

int main(void)
{
    int i;

    #if defined (__USE_LPCOPEN)
    #if !defined(NO_BOARD_LIB)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
	#else
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, PORT, BIT);
    #endif
    #endif

    while(1)
    {
		#if defined (__USE_LPCOPEN)
		#if !defined(NO_BOARD_LIB)
    	Board_LED_Toggle(0);
        for(i = 0; i < 2000000; i++)
        {
            __asm__("nop");
        }
		#else
        Chip_GPIO_SetPinState(LPC_GPIO_PORT, PORT, BIT, true);
        for(i = 0; i < 2000000; i++)
        {
            __asm__("nop");
        }
        Chip_GPIO_SetPinState(LPC_GPIO_PORT, PORT, BIT, false);
        for(i = 0; i < 2000000; i++)
        {
            __asm__("nop");
        }
		#endif
		#endif
    }

    return 0;
}
