#ifndef		SPI_INIT_H
#define		SPI_INIT_H

#include <stdint.h>

// MCP23S17 IC Address (BANK = 0)
#define IODIRA		0x00
#define IODIRB		0x01
#define IPOLA			0x02
#define IPOLB			0x03
#define GPINTENA	0x04
#define GPINTENB	0x05
#define DEFVALA		0x06
#define DEFVALB		0x07
#define INTCONA		0x08
#define INTCONB		0x09
#define IOCONA		0x0A
#define IOCONB		0x0B
#define GPPUA			0x0C
#define GPPUB			0x0D
#define INTFA			0x0E
#define INTFB			0x0F
#define INTCAPA		0x10
#define INTCAPB		0x11
#define GPIO_A		0x12
#define GPIO_B		0x13
#define OLATA			0x14
#define OLATB			0x15

// Opcodes for MotorControlled IC(A0 = A1 = 0)
#define MCP_WRITE_OPCODE	0x40
#define MCP_READ_OPCODE		0x41



void SPI_init();
void CS_LOW();
void CS_HIGH();
uint8_t spi_txrx(uint8_t data);
void drain_rx_clear_ovr();
void write_reg(uint8_t address, uint8_t data);
uint8_t read_reg(uint8_t address);
void MCP_init();

#endif