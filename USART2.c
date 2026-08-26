// Communication for terminal(Not Needed)
#include "USART2.h"
#include "stm32f446xx.h"
#include <stdint.h>

void USART2_init()
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;		// Enable PortA clock
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;		// Enable USART2 clock 
	
	//PA3(RX) & PA2(TX) Alt Func
	GPIOA->MODER &= ~((0x3 << (2 * 3)) | (0x3 << (2 * 2)));	// Clear Pin mode
	GPIOA->MODER |=  ((0x2 << (2 * 3)) | (0x2 << (2 * 2)));	// Alt Function mode
	
	//PA3[15:12] & PA2[11:8] ALT Func
	GPIOA->AFR[0] &= ~0xFF00;		// Clear Pin mode
	GPIOA->AFR[0] |=  0x7700;		// Alt7 for USART2 
	
	// fck = 16 MHz. Desired Baud Rate(DBR) = 115200.
	// BRR = fck / (8OverSamplingSize * DBR) = 16Mhz / (8*115200) = 17.3611
	// int Baud[3:1] = 17 = 0x011x.
	// fr Baud[0] = (0.3611 * 8) + 0.5 = 3.388 = 3
	// BRR = 0x0113
	
	USART2->CR1 |= (1 << 15);		// Oversampling of 8 bits
	USART2->BRR = 0x0113; 			// Baud Rate: 115200 bps @16 MHz clk
	USART2->CR1 |= 0x0008;			// enable Tx, 8-bit data 
	USART2->CR1 |= 0x0004;			// enable Rx, 8-bit data 
	USART2->CR2 = 0x0000; 			// 1 stop bit
	USART2->CR3 = 0x0000;				// no flow control
	USART2->CR1 |= USART_CR1_UE;// USART Enabled
}


// Write a Character to USART2(terminal)
void putcUSART2(char ch)	// Print character from USART
{
	while (!(USART2->SR & USART_SR_TXE)) {} // wait until Tx buffer empty Tx[0] = Full; Tx[1] = Empty
	USART2->DR = ch;
}
// Write CharacterS to USART2(terminal)
void putsUSART2(char *str)
{
	uint8_t i = 0;
	while(str[i] != '\0')
		putcUSART2(str[i++]);
}


// Read a Character from USART2(terminal)
char getcUSART2(void)		// Get a character from USART
{
	while (!(USART2->SR & USART_SR_RXNE)) {} // wait until char arrives. RXNE[0] = Data not received; RXNE[1] = Received.
	return USART2->DR;
}
