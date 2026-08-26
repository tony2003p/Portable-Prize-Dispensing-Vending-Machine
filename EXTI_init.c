#include "stm32f446xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "EXTI_init.h"
#include "SPI_INIT.h"
#include "STATE.h"
#include "GPIO_init.h"
#include "USART2.h"

#define IC_INTERRUPT_PIN 2
#define TIM_CRI_OPM		TIM_CR1_OPM_Msk

	
// LED to Toggle in Interrupt(NOT NEEDED)
#define PC13_msk	0x2000
//NOT NEEDED
#define PC1_msk 	0x2

volatile uint8_t intf = 0;
volatile uint8_t motorBit = 0;
volatile char MotorControl;

void NVIC_Button_init()
{
	/**** GPIO Setup ****/
	// 1. Enable CLK; 2. Set pin as Input; 3. Set PUPDR;
	/**** Configure interrupt Push-button PC0(pull-up) on EXTI0 ****/	
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	SYSCFG->EXTICR[2] &= ~0xF;		// PC8[3:0] is on EXIT8
	SYSCFG->EXTICR[2] |=	0x2;		// PC0[3:0], 0010 for C. A=0, B=1, C=2, D=3....
	EXTI->IMR |= PC8_msk;			// Enable Interrupt request
	EXTI->RTSR &= ~PC8_msk;		// Disable Rising trigger
	EXTI->FTSR |= PC8_msk;		// Falling Edge trigger

	//Enable Interrupt on NVIC for EXTI3	
	NVIC_SetPriority(EXTI9_5_IRQn, IC_INTERRUPT_PIN);	// (IRQ name, Priority#)
	NVIC_EnableIRQ(EXTI9_5_IRQn);											// (IRQ name)  Enabled
}

void EXTI9_5_IRQHandler()
{	
	if(EXTI->PR & PC8_msk)
	{
		EXTI->PR |= PC8_msk;		// Clear interrupt pending
		EXTI->IMR &= ~PC8_msk;	// Disable button interrupt
		
		PC1Low();	/***** NOT NEED> USED FOR DEBUGGING *****/
		
		/***** IC Interrupt Control *****/
		// read interrupt flag from MCP to clear	
		intf = read_reg(INTCAPB);		// Check which pins triggered interrupt
		
		// Clear Interrupt. DONE IN TIM2 HANDLER
		switch(MotorControl)
		{
			case '0':
				motorState = (intf == 0x3E) ? SYS_SUCCESS : SYS_FAILURE;
				break;
			case '1':
				motorState = (intf == 0x3D) ? SYS_SUCCESS : SYS_FAILURE;
				break;
			case '2':
				motorState = (intf == 0x3B) ? SYS_SUCCESS : SYS_FAILURE;
				break;
			case '3':
				motorState = (intf == 0x37) ? SYS_SUCCESS : SYS_FAILURE;
				break;
			case '4':
				motorState = (intf == 0x2F) ? SYS_SUCCESS : SYS_FAILURE;
				break;	
			case '5':
				motorState = (intf == 0x1F) ? SYS_SUCCESS : SYS_FAILURE;
				break;
			default:
				motorState = SYS_FAILURE;
		}
		
		if(motorState == SYS_FAILURE)
			printf("Miss Match interrupt to motor control!!!\n\n\r");
		
	}
}

