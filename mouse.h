/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 1989-2008 ARC International
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
* 
* @brief The file contains Macro's and functions needed by the mouse 
*        application
* 
*END************************************************************************/


#ifndef _MOUSE_H
#define _MOUSE_H

#include "usb_descriptor.h"

/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/
#define  MOUSE_BUFF_SIZE   (4)   /* report buffer size */
#define  REQ_DATA_SIZE     (1)

#define COMPLIANCE_TESTING    (0)/*1:TRUE, 0:FALSE*/
/******************************************************************************
 * Types
 *****************************************************************************/
typedef struct _mouse_variable_struct
{
    HID_HANDLE app_handle;
    bool mouse_init;/* flag to check lower layer status*/
    uint8_t rpt_buf[MOUSE_BUFF_SIZE];/*report/data buff for mouse application*/
    USB_CLASS_HID_ENDPOINT ep[HID_DESC_ENDPOINT_COUNT];
    uint8_t app_request_params[2]; /* for get/set idle and protocol requests*/
}MOUSE_GLOBAL_VARIABLE_STRUCT, * PTR_MOUSE_GLOBAL_VARIABLE_STRUCT; 

typedef struct mouse_settings_struct
{
	int MOUSE_DIR;
	int MOUSE_SECOND_DIR;
	int MOUSE_SPEED;
	int SECOND_PRESS;
	int BUTTON;
	int BUTTON_PRESS;
} Mouse_settings;

/*****************************************************************************
 * Global variables
 *****************************************************************************/

/*****************************************************************************
 * Global Functions
 *****************************************************************************/
extern void TestApp_Init(void);

void sendMouseCommand(Mouse_settings ms);
void initMouseSpeed(Mouse_settings *ms, int speed);
void initMouseSettings(Mouse_settings *ms);
int activeHigh();
int initButton(LWGPIO_STRUCT *btn, int button);
int buttonPressed(LWGPIO_STRUCT *btn);
void setDir(Mouse_settings *ms, int DIR);
void printWelcome(Mouse_settings *ms);
void print(char *string);
int buttonPressedAndReleased(LWGPIO_STRUCT *btn, int *btn_state, int *btn_last_state);
void updateButtonState(LWGPIO_STRUCT *btn, int *btn_state);
void printSettings(Mouse_settings *ms);

#endif 

/* EOF */
