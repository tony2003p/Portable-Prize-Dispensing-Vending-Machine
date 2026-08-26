#include "stm32f446xx.h"
#include <stdint.h>
#include "TIM2_init.h"
#include "SPI_INIT.h"
#include "STATE.h"

#define PSC_500HZ				32000 - 1			// 16MHz/32KHz = 500Hz			1/500Hz = 2ms per tick
#define WAIT_TIME				10000 - 1			// 20 sec timer (10k * 2ms = 20sec)
#define PRIORITY_TIM2   3

// Button Interrupt Input
#define PC8_msk 0x100

volatile bool TimeOutFailure = false;

void TIM2_init()
{
	RCC->APB1ENR |= 0x1;		// Enable clock for TIM2
	
	/** setup TIM2: example from p6_7.c **/
	TIM2->PSC = PSC_500HZ; 				// 2ms 
	TIM2->ARR = WAIT_TIME ;				// 20 sec 
	TIM2->EGR = TIM_EGR_UG;
	TIM2->DIER |= 1; 							/* enable UIE */
	NVIC_SetPriority(TIM2_IRQn, PRIORITY_TIM2);
	NVIC_EnableIRQ(TIM2_IRQn); 		/* enable interrupt in NVIC */
}

/* ===== Delays for Debounce (30 ms) and the enables ISR again ===== */
void TIM2_IRQHandler()
{	// Tim#->SR(status register) UIF(update interrupt flag)
	if (TIM2->SR & TIM_SR_UIF) 	
	{
		TIM2->SR &= ~TIM_SR_UIF;  // clear update flag (acknowledge)
		EXTI->IMR |= PC8_msk;			// enables interrupt on PC13 again
		read_reg(INTFB);					// Acknowledge and clear Interrupt from MCP.
		
		if(motorState == SYS_DISPENSING)	// Still attempting to dispense(failed)
		{
			motorState = SYS_FAILURE;
		}
  }
}	