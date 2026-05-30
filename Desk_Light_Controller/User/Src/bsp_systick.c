#include "bsp_systick.h"

//global variables
static uint32_t IntCounter; //Count how many times intertupts are triggered

/* parameter SystemCoreClock/1000    1ms per interrupt
 *           SystemCoreClock/100000  10us per interrupt
 *           SystemCoreClock/1000000 1us per interrupt
 */
void Systick_Init(void)
{
 if(SysTick_Config(SystemCoreClock/100000))
 {
		while(1); // If something goes wrong with initialization, go into this loop
 }
}

void Delay_10us(uint32_t nTime)
{
	IntCounter = nTime;
	while (IntCounter != 0);
}

void IntCounterDecrement(void)
{
	if (IntCounter!= 0) //Without this, IntCounter can go to -1
	{
		IntCounter--;
	}
	
}
