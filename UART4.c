// Communication between STM32 and Raspberry Pi
#include "UART4.h"
#include "stm32f446xx.h"
#include <stdint.h>
#include <stdbool.h>

#define PA1_msk		0x2
#define RX_INTERRUPT 1

volatile bool dataReceived = false;

void UART4_init()
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;		// Enable PortA clock
	RCC->APB1ENR |= RCC_APB1ENR_UART4EN;		// Enable UART4 clock 
	
	//PA1(RX) & PA0(TX) Alt Func
	GPIOA->MODER &= ~((0x3 << (2 * 1)) | (0x3 << (2 * 0)));	// Clear Pin mode
	GPIOA->MODER |=  ((0x2 << (2 * 1)) | (0x2 << (2 * 0)));	// Alt Function mode
	
	//PA1[7:4] & PA0[3:0] ALT Func
	GPIOA->AFR[0] &= ~0x00FF;		// Clear Pin mode
	GPIOA->AFR[0] |=  0x0088;		// Alt8 for UART4 
	
	// fck = 16 MHz. Desired Baud Rate(DBR) = 115200.
	// BRR = fck / (8OverSamplingSize * DBR) = 16Mhz / (8*115200) = 17.3611
	// int Baud[3:1] = 17 = 0x011x.
	// fr Baud[0] = (0.3611 * 8) + 0.5 = 3.388 = 3
	// BRR = 0x0113

	UART4->CR1 |= (1 << 15);		// Oversampling of 8 bits
	UART4->BRR = 0x0113; 			// Baud Rate: 115200 bps @16 MHz clk
	UART4->CR1 |= 0x0008;			// enable Tx, 8-bit data 
	UART4->CR1 |= 0x0004;			// enable Rx, 8-bit data 
	UART4->CR2 = 0x0000; 			// 1 stop bit
	UART4->CR3 = 0x0000;			// no flow control
	UART4->CR1 |= USART_CR1_RXNEIE;	// RX Interrupt Enabled
	UART4->CR1 |= USART_CR1_UE;		// UART Enabled
	
	//Enable Interrupt on NVIC for UART4_IRQn
	NVIC_SetPriority(UART4_IRQn, RX_INTERRUPT);	// (IRQ name, Priority#)
	NVIC_EnableIRQ(UART4_IRQn);					// (IRQ name)  Enabled
}

void UART4_IRQHandler()
{
	if(UART4->SR & USART_CR1_RXNEIE)
	{		
		dataReceived = true;
	}
}


// Write a Character to UART4(Raspberry Pi)
void putcUART4(char ch)	// Print character from UART
{
	while (!(UART4->SR & USART_SR_TXE)) {} // wait until Tx buffer empty Tx[0] = Full; Tx[1] = Empty
	UART4->DR = ch;
}
// Write CharacterS to UART4(Raspberry Pi
void putsUART4(char *str)
{
	uint8_t i = 0;
	while(str[i] != '\0')
		putcUART4(str[i++]);
}

// Read a Character from UART4(terminal)
char getcUART4(void)		// Get a character from Raspberry Pi
{
	while (!(UART4->SR & USART_SR_RXNE)) {} // wait until char arrives. RXNE[0] = Data not received; RXNE[1] = Received.
	return UART4->DR;
}
