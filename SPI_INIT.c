#include "stm32f446xx.h"
#include <stdint.h>
#include "SPI_INIT.h"
#include "GPIO_init.h"
#include "util.h"

#define MSTR	(1 << 2)	// Master Enabled - Bit 2 
#define BR		(6 << 3)	// Baud Rate - Bit[5:3]	FPCLK/128
#define SPE		(1 << 6)	// SPI Enabled - Bit 6
#define SSM		(1 << 9)	// Slave Select Enabled - Bit 9
#define SSI		(1 << 8)	// SSM enabled SSI = 1 - Bit 8


void SPI_init()
{
	/** 1. Enable clock **/
	RCC->APB2ENR |= (1 << 12);     // Enable SPI1 clk

	/** 2. Configure PA5(SCK), PA6(MISO), PA7(MOSI) for Alternative function. **/
		// Done in GPIO_init

	/** 3. Configure SPI1 **/
		// reset first
		// BR. Baud Rate = fPCLK/256
		// MSTR. Master mode selection
		// SSM=1, SSI=1, Software controls which slave to use
		// CPOL=0, CPHA=0, sample on 1st rising edge
		// DFF=0, 8-bit frame
	SPI1->CR1 = 0;	// Reset
	SPI1->CR2 = 0;	// Reset
	SPI1->CR1 |= MSTR | BR | SSM | SSI;	// Configure SPI functions
	
	SPI1->CR1 |= SPE;		// Enabled SPI
}

void CS_LOW() // Clear PA4 for CS low
{	GPIOA->BSRR = (1 << (4 + 16)); }

void CS_HIGH() // Set PA4 for CS high
{ GPIOA->BSRR = (1 << 4);	}


uint8_t spi_txrx(uint8_t data)
{
	// Wait until the tx buffer is empty
	while (!(SPI1->SR & (1U << 1))) { /* spin */ }

	// Write data
	*((volatile uint8_t *)&SPI1->DR) = data; // write 8-bit to DR

	// Wait until the rx buffer is full
	while (!(SPI1->SR & (1U << 0))) { /* spin */ }

	// Read received data
	uint8_t rx = *((volatile uint8_t *)&SPI1->DR);

	// If OVR flag set, clear it by reading DR then SR (we already read DR; read SR)
	if (SPI1->SR & (1U << 6)) // OVR is bit 6
	{
		volatile uint32_t tmp = SPI1->DR;
		(void)tmp;
		tmp = SPI1->SR;
		(void)tmp;
	}
	return rx;
}

void drain_rx_clear_ovr()
{
	// Read DR/RR while RXNE and then SR to clear OVR if set
	while (SPI1->SR & (1U << 0))
	{
		volatile uint8_t tmp = *((volatile uint8_t *)&SPI1->DR); // Dump
		(void)tmp; // Not used. Gets rid of warning
	}
	volatile uint32_t tmp = SPI1->SR;	// Dump
	(void)tmp;	// Not used. Gets rid of warning
}

void write_reg(uint8_t address, uint8_t data)
{
    CS_LOW();
    spi_txrx(MCP_WRITE_OPCODE); // opcode
    spi_txrx(address);
    spi_txrx(data);
    drain_rx_clear_ovr();
    CS_HIGH();
}

uint8_t read_reg(uint8_t address)
{
    uint8_t read;
    CS_LOW();
    spi_txrx(MCP_READ_OPCODE);
    spi_txrx(address);
    read = spi_txrx(0xFF); // sending dummy data to start clk
    drain_rx_clear_ovr();
    CS_HIGH();
    return read;
}

void MCP_init()
{
	//disable reset
	PC6High();
	delayMs(100); //wait for reset to settle

	// Bank = 0; SEQOP = 1(disabled sequential operation); 
	// HAEN = 1(enabled addresses); ODR = 1(opned-drain)
	// INTPOL = 0(Active-Low)
	write_reg(IOCONA, 0x28);	
	write_reg(IOCONB, 0x28);	// Bank = 0; Mirror = 0; DISSLW = 1; HAEN = 1; Open Drain = 1;
	
	/***** Initialization for MotorControl Setup *****/
	write_reg(IODIRA, 0x00);	// All setup as outputs (0)
	write_reg(OLATA, 0x00);		// Clear OLAT outputs
	
	volatile uint8_t gppub_val = 0x4;
	
	/***** Initialization for Phototransistor Setup *****/
	write_reg(IODIRB, 0x1F);	// All Inputs (1)
	write_reg(GPPUB, 0x00);		// Pull-up disabled
	write_reg(OLATB, 0x1F);		// Clear OLAT outputs
	
	// do{
	// 	gppub_val = read_reg(GPIO_B);
	// }while(gppub_val != 0x1F);
	
	
	/**** MCP Button Interrupt Setup ****/
	write_reg(GPINTENB, 0x1F);	// Enabled interrup [4:0]
	// Pin compared to default
	write_reg(INTCONB, 0x1F);		// Pin Compared to default
	// ODR(bit2) = 0, Active Driver Output.   INTPOL(bit1) = 0, Active-low.
	write_reg(DEFVALB, 0x1F);		// Default Value (1) for [5:0]
}


