#include "chip.h"

#include <cr_section_macros.h>

// Pin Name: P1_1
#define PORT 	0
#define BIT		8

int main(void)
{
	int i;
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, PORT, BIT);

    while(1)
    {
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
    }

    return 0;
}
