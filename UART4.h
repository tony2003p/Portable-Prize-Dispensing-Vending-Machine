#ifndef UART4_H
#define UART4_H

#include <stdbool.h>
volatile extern bool dataReceived;

void UART4_init();
void NVIC_RX_init();
void EXTI1_IRQHandler();
void putcUART4(char ch);
void putsUART4(char *str);
char getcUART4(void);

#endif
