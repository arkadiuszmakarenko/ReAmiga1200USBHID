/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/09/01
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Azftware (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 * @Note
 * This example demonstrates the process of enumerating the keyboard and mouse
 * by a USB host and obtaining data based on the polling time of the input endpoints
 * of the keyboard and mouse.
 * The USBFS port also supports enumeration of keyboard and mouse attached at tier
 * level 2(Hub 1).
 */

/*
 * @Note
 * The USBFS module uses the system clock as the clock source, so the SystemCoreClock can
 * only be set to 144MHz, 96MHz or 48MHz.
 */

/*******************************************************************************/
/* Header File */
#include "utils.h"
#include "tim.h"
#include "mouse.h"
#include "gpio.h"
#include "keyboard.h"
#include "usb_device_handler.h"

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main (void) {
    /* Initialize system configuration */
    Delay_Init();
    USART_Printf_Init (115200);
    DUG_PRINTF ("SystemClk:%d\r\n", SystemCoreClock);

    /* Initialize TIM3 */
    TIM3_Init (9, SystemCoreClock / 10000 - 1);
    DUG_PRINTF ("TIM3 Init OK!\r\n");

    /* Initialize USBFS host */
    usb_init();

    /* Initialize peripherals */
    DUG_PRINTF ("Initializing TIM2...\r\n");
    TIM2_Init();
    DUG_PRINTF ("TIM2 Init OK\r\n");
    
    DUG_PRINTF ("Initializing TIM4...\r\n");
    TIM4_Init();
    DUG_PRINTF ("TIM4 Init OK\r\n");
    
    DUG_PRINTF ("Initializing GPIO...\r\n");
    GPIO_Config();
    DUG_PRINTF ("GPIO Init OK\r\n");
    
    DUG_PRINTF ("Initializing Mouse...\r\n");
    InitMouse();
    DUG_PRINTF ("Mouse Init OK\r\n");
    
    /* Initialize Amiga keyboard interface */
    DUG_PRINTF ("Initializing Amiga keyboard...\r\n");
    amikb_startup();
    DUG_PRINTF ("Amiga keyboard initialized\r\n");
    
    DUG_PRINTF ("All systems initialized - entering main loop\r\n");

    while (1) {
        usb_process_devices();
    }
}
