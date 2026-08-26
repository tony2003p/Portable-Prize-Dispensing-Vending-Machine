#include "stm32f446xx.h"
#include <stdint.h>
#include "GPIO_init.h"

#define PA4 3 << (4*2)  // 0x300		CS
#define PA5	3 << (5*2)  // 0xC00		SCLK
#define PA6 3 << (6*2)  // 0x3000		MISO
#define PA7 3 << (7*2)  // 0xC000		MOSI

#define PC8_msk 0x100
#define PC1	 3 << (1*2)	// 0xC
	
// IC PIN Interrupt Input
#define PC0_msk		0x1		
//NOT NEEDED
#define PC1_msk 	0x2

void GPIO_init()
{
	RCC->AHB1ENR |= (1 << 0);			// Enable PortA clk
	/**** Configure PA4 (CS) - output, ****/
	/**** Configure PA5(SCK), PA6(MISO), PA7(MOSI) for Alternative function. PA2 CS Output ****/
	GPIOA->MODER &= ~(PA4 | PA5 | PA6 | PA7);
	GPIOA->MODER |= (1 << (4*2));					// PA4(out)
	GPIOA->MODER |=  ((2 << (5*2)) | (0x2 << (6*2)) | (0x2 << (7*2)));			// ALT (01) for 5-7
	GPIOA->AFR[0] &= ~((0xF << (5*4)) | (0xF << (6*4)) | (0xF << (7*4)));		// ALT Mode Textbook 301
	GPIOA->AFR[0] |=  ((0x5 << (5*4)) | (0x5 << (6*4)) | (0x5 << (7*4)));		// ALT Mode Textbook 301
	
	// Set PA4 HIGH to start CS HIGH
	GPIOA->BSRR = (1 << 4);
	
	/**** PC6 RESET for MCP ****/ // set high to reset
	RCC->AHB1ENR |= (1 << 2);		// Enable PortC clk
	GPIOC->MODER &= 0x3000;			// Clear pin mode for PC6
	GPIOC->PUPDR |= 0x1 << 2*6;     // Pull-up (disable reset)
	GPIOC->MODER |= 0x1000;			// Set PC6 as output(01)
	
	/**** PC8 for Button Interrupt. ****/
	GPIOC->MODER &= ~PC8_msk;						// Clear pin modes. PC0 set as input
	GPIOC->PUPDR &= ~(3 << (8 * 2));		// Clear pull bits
	GPIOC->PUPDR |= (1 << (8 * 2));			// PC8 Set pull-up (01)
	
	//Just for me to physically test
	GPIOC->MODER &= ~(PC1);
	GPIOC->MODER |= 1 << (1*2);

}

void PC6High()	// --- FOR TESTING
{
	GPIOC->BSRR |= 0x40;
}
void PC6Low()		// --- FOR TESTING
{
	GPIOC->BSRR |= 0x400000;
}

void PC1High()
{
	GPIOC->BSRR |= 0x2;
}

void PC1Low()
{
	GPIOC->BSRR |=0x2000;
}