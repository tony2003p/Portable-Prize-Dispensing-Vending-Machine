/**************************************************************
* SPI -> IC -> Motors and Phototransistors
*	PA4: A2			CS
* PA5: D13		SCLK
* PA6: D12		MISO
* PA7: D11		MOSI
* PC6: PC6 -> !RESET(3.3V)
* PC8:				Interrupt Detection Pin
* A2 = `A1 = 0V; A0 = 0V
* GPIOA[4:0]	MotorDrivers
* GPIOB[4:0]	Phototransistors
**************************************************************/

/**************************************************************
* USART2 to verify on terminal(Not Needed for Final project)
* PA2: D1			TX
* PA3: D0			RX
**************************************************************/

/**************************************************************
* UART4 Communication between Raspberry Pi
* PA0: A0			TX
* PA1: A1			RX
**************************************************************/

/**************************************************************
* LED Testing debugging purposes
* PA9: D8			LED output For TIMER failure
* PC1: A4			LED output For Interrupt Pin
**************************************************************/

#include "stm32f446xx.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "SPI_INIT.h"
#include "GPIO_init.h"
#include "TIM2_init.h"
#include "EXTI_init.h"
#include "STATE.h"
#include "util.h"

#include "HardwareComms.h"

volatile SystemState motorState = SYS_IDLE;
//volatile uint8_t motorBit = 0;


//return true if motor control is done, false if it needs to keep going
MotorReturnValue processMotorControl(uint8_t MotorControl)
{
	MotorReturnValue retVal = {0};
	retVal.isDone = false;
	retVal.statusCode = STATUS_CODE_OK;

	switch(motorState)
		{
			case SYS_IDLE:

				//printf("\n\rEnter 5 to RESET the MCP\n\r");
				//printf("\nEnter Motor Value (0-4): ");
		
				//while(1);
				//printf("\n\r");

				if(MotorControl < 0 || MotorControl > 5)	// Validates motor/reset input
				{
					printf("\n\rInvalid input. Enter 0-5.\n\r");
					motorState = SYS_IDLE;
					retVal.isDone = true;
				}
				else if(MotorControl == 5)
				{
					printf("\n\rRESETTING MCP!!!\n\n\r");
					PC6Low(); //enable reset
					delayMs(500000);
					PC6High(); //disable reset
					motorState = SYS_IDLE;
					retVal.isDone = true;
				}
				else
				{
					printf("\n\rValid input received. Starting motor...\n\r");
					motorState = SYS_START_MOTOR;
					retVal.isDone = false;
				}
				break;
				
			case SYS_START_MOTOR:
				motorBit = 1 << (MotorControl); 			// Sets char to motor int
				write_reg(OLATA, motorBit);	// Turns on selected motors
				
				// Reset flags and arm the timeout timer
				//waiting = true;
				//dispenseSuccess = false;
				//TimeOutFailure = false;

				TIM2->CR1 = 0;           	// Disable timer
				TIM2->CNT = 0;           	// Reset counter
				TIM2->CR1 |= TIM_CR1_OPM; // One-pulse mode
				TIM2->CR1 |= TIM_CR1_CEN; // Enable TIM2

				// Enable MCP interrupt for phototransistors
				EXTI->PR = PC8_msk;       // Clear any pending edge
				EXTI->IMR |= PC8_msk;     // Re-arm EXTI
			
				read_reg(INTCAPB);  // Clear interrupt on MCP
				
				motorState = SYS_DISPENSING;

				retVal.isDone = false;
				break;
				
			case SYS_DISPENSING:
				// Waits for either Dispensing Interrupt from IC or Timer Interrupt		
				PC1High();		// LED turns OFF if pin ISR triggers

				retVal.isDone = false;
				break;
			
			case SYS_SUCCESS:
				TIM2->CR1 = 0;   // Disable timeout timer
			
				write_reg(OLATA, 0x00);	// Turn OFF motors after success

				// Re-arm EXTI for next operation
				EXTI->PR = PC8_msk;
				EXTI->IMR |= PC8_msk;

				//putcUART4('0'); /***** Notify Raspberry Pi success *****/
				printf("\n\rSuccessfully Dispensed Item\n\r");	
				
				motorState = SYS_IDLE;

				retVal.isDone = true;
				retVal.statusCode = STATUS_CODE_OK; //OK
				break;
			
			case SYS_FAILURE:
				write_reg(OLATA, 0x00);	// Turn OFF motors after timeout
			
				TIM2->CR1 = 0;					// Disable timer
				//putcUART4('1');		/***** Notify Raspberry Pi failure *****/
				printf("\n\rFailed to Dispense Item (Timeout)\n\r");
			
				motorState = SYS_IDLE;

				retVal.isDone = true;
				retVal.statusCode = STATUS_CODE_FAILURE_TIMEOUT; //TIMEOUT FAILURE
				break;
		}

		return retVal;
}