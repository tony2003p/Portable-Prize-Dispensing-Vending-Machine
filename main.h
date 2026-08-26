/**
  ******************************************************************************
  * @file    USB_Host/AOA_Standalone/Inc/main.h 
  * @author  MCD Application Team
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f446xx.h"

#include "stdio.h"
#include "usbh_core.h"
#include "usbh_aoa.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef enum {
  AOA_SEND_IDLE = 0,
  AOA_SEND_WAIT,  
}AOA_Send_State;

typedef enum {
  AOA_RECEIVE_IDLE = 0,
  AOA_RECEIVE_WAIT,    
  AOA_RECEIVE_RECEIVE,
}AOA_Receive_State;

typedef enum {
  AOA_SELECT_MENU = 0,
  AOA_SELECT_FILE ,  
  AOA_SELECT_CONFIG,    
}AOA_DEMO_SelectMode;

typedef enum {
  APPLICATION_IDLE = 0,
  APPLICATION_DISCONNECT,  
  APPLICATION_START,
  APPLICATION_READY,    
  APPLICATION_RUNNING,
} AOA_ApplicationTypeDef;

typedef enum {
  CONTROL_IDLE,
  CONTROL_RUNNING
} ControlState;

extern USBH_HandleTypeDef hUSBHost;
extern AOA_ApplicationTypeDef Appli_state;
/* Exported constants --------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void AOA_Handle_Send_Menu(void);
void AOA_Handle_Receive_Menu(void);
void AOA_MenuProcess(void);

#endif /* __MAIN_H */
