#ifndef	EXTI_INIT_H
#define	EXTI_INIT_H


#include <stdint.h>

// MCP Interrupt Pin - Input
#define PC8_msk 0x100

volatile extern uint8_t motorBit;
volatile extern char MotorControl;

void NVIC_Button_init();
void EXTI9_5_IRQHandler();

#endif