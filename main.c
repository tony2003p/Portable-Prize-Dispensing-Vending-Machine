/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2016 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stdio.h>

#include "Board_LED.h"                  // ::Board Support:LED
#include "Board_Buttons.h"              // ::Board Support:Buttons

#include "stm32f4xx.h"                  // Device header
#include "stm32f446xx.h"

#include "usbh_core.h"
#include "usbh_aoa.h"

#include "SPI_INIT.h"
#include "GPIO_init.h"
#include "TIM2_init.h"
#include "EXTI_init.h"
#include "STATE.h"
#include "util.h"
#include "HardwareComms.h"

#include "main.h"

//Magic number as first byte of payloads
#define MAGIC_NUMBER 0xA8

extern int stdout_init (void);
USBH_HandleTypeDef hUSBHost;
AOA_ApplicationTypeDef appState = APPLICATION_IDLE;

/*----------------------------------------------------------------------------
 * SysTick_Handler:
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * SystemCoreClockConfigure: configure SystemCoreClock using HSE
                             (HSE is populated on Nucleo board)
 *----------------------------------------------------------------------------*/
void SystemCoreClockConfigure(void) {

  //From STM32F446ZE-Nucleo USB Host example

  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);


  //HSE = 8MHz (from ST-LINK MCO clock output)
  //PLL input after div = 8MHz/PLLM =8MHz/8 = 1MHz
  //VCO = HSE*PLLN/PLLM = 8MHz * 360/8 = 360MHz
  //PLL out =  VCO/PLLP = 360MHZ/2 = 180MHz
  //System clock = PLL out = 180MHz
  // USB OTG FS = = VCO/PLLQ = 360MHz/7 = 51.42MHz (target = 48MHz) <- NOT USED

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Activate the OverDrive to reach the 180 MHz Frequency */
  HAL_PWREx_EnableOverDrive();

  //PLLSAI
  //Input = HSE = 8MHz
  //VCO = HSE * PLLSAIN/PLLSAIM = 8MHz * 384/8 = 384MHz
  //48 MHz out (fed to USB FS) = VCO/PLLP = 384MHz/8 = 48MHz

  /* Select PLLSAI output as USB clock source */
  PeriphClkInitStruct.PLLSAI.PLLSAIM = 8;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 384;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV8;
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CK48;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CK48CLKSOURCE_PLLSAIP;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1; //180MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4; //45MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2; //90MHz
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

/**
  * @brief  User Process
  * @param  phost: Host Handle
  * @param  id: Host Library user message ID
  * @retval None
  */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
  switch(id)
  { 
  case HOST_USER_SELECT_CONFIGURATION:
    break;
    
  case HOST_USER_DISCONNECTION:
    appState = APPLICATION_DISCONNECT;
    break;
    
  case HOST_USER_CLASS_ACTIVE:
    appState = APPLICATION_READY;
    break;
    
  case HOST_USER_CONNECTION:
    //appState = APPLICATION_START;
    break;

  case HOST_USER_UNRECOVERED_ERROR:
    printf("ERROR in USB Host process, exiting.\n");
    //exit(0);
    break;
    
  default:
    break; 
  }
}

/*----------------------------------------------------------------------------
 * main: blink LED and check button state
 *----------------------------------------------------------------------------*/
int main (void) {

  HAL_Init();

  SystemCoreClockConfigure();                              /* configure HSE as System Clock */
  SystemCoreClockUpdate();

  //HW init
  LED_Initialize();
  Buttons_Initialize();
  stdout_init();                                           /* Initialize Serial interface */

  //Motor Control System init
  GPIO_init();
	SPI_init();
	NVIC_Button_init();
	TIM2_init();
	MCP_init();

  //USB initialize
  USBH_Init(&hUSBHost, USBH_UserProcess, 0);
  USBH_RegisterClass(&hUSBHost, USBH_AOA_CLASS);
  USBH_Start(&hUSBHost);

  printf("Initialized!\n");

  //Motor test init
	// write_reg(OLATA, 0xFF);		delayMs(250);	
	// write_reg(OLATA, 0x00);		delayMs(250);
	// write_reg(OLATA, 0x01);		delayMs(250);
	// write_reg(OLATA, 0x02);		delayMs(250);
	// write_reg(OLATA, 0x04);		delayMs(250);
	// write_reg(OLATA, 0x08);		delayMs(250);
	// write_reg(OLATA, 0x10);		delayMs(250);
	// write_reg(OLATA, 0x20);		delayMs(250);	
	// write_reg(OLATA, 0x00);		delayMs(250);

  char buff[AOA_TX_BUFFER_SIZE] = "hello there!";

  //USB Receive buffer
  uint8_t commandBuffer[2] = {0};
  uint16_t commandMsgSize = 0;

  //USB response buffer
  uint8_t responseBuffer[2] = {MAGIC_NUMBER, 0}; //first byte is always magic number
  uint16_t responseMsgSize = 2;

  ControlState controlState = CONTROL_IDLE;

  MotorReturnValue motorReturn = {0};

  while (1)
  {
    //Process USB events
    USBH_Process(&hUSBHost);

    //handle incoming dispense requests

    //read incoming data
    switch (controlState)
    {
      case CONTROL_IDLE:
        //wait for command
        commandMsgSize = USBH_AOA_read(&hUSBHost, commandBuffer, 2);

        if (commandMsgSize > 0)
        {
          printf("Got data: %d %d" , commandBuffer[0], commandBuffer[1]);
        }

        if (commandMsgSize >= 2 && commandBuffer[0] == MAGIC_NUMBER)
        {
          controlState = CONTROL_RUNNING;
          commandMsgSize = 0;
        }
        break;

      case CONTROL_RUNNING:
        motorReturn = processMotorControl(commandBuffer[0]);

        //run motor control state machine until it is finished
        if (motorReturn.isDone)
        {
          //send USB dispense status code
          responseBuffer[1] = motorReturn.statusCode;
          USBH_AOA_write(&hUSBHost, responseBuffer, responseMsgSize);

          controlState = CONTROL_IDLE;
        }

        break;
    }
  }

}
