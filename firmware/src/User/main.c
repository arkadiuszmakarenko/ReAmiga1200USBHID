/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/09/01
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
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
#include <usb_gamepad.h>
#include <usb_mouse.h>
#include <usb_keyboard.h>
#include "usb_host_config.h"
#include "utils.h"
#include "tim.h"
#include "mouse.h"
#include "gpio.h"
#include "keyboard.h"
#include "gamepad.h"

void USARTx_CFG (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_USART1, ENABLE);

    /* USART2 TX-->A.2   RX-->A.3 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init (GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    USART_InitStructure.USART_BaudRate = 2400;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init (USART1, &USART_InitStructure);
    USART_Cmd (USART1, ENABLE);
}

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
    // USART_Printf_Init (115200);
    // DUG_PRINTF ("SystemClk:%d\r\n", SystemCoreClock);
    // DUG_PRINTF ("USBFS HOST KM Test\r\n");

    /* Initialize TIM3 */
    TIM3_Init (9, SystemCoreClock / 10000 - 1);
    // DUG_PRINTF ("TIM3 Init OK!\r\n");
    USARTx_CFG();

    /* Initialize USBFS host */
#if DEF_USBFS_PORT_EN
    // DUG_PRINTF ("USBFS Host Init\r\n");
    USBFS_RCC_Init();
    USBFS_Host_Init (ENABLE);
    memset (&RootHubDev.bStatus, 0, sizeof (ROOT_HUB_DEVICE));
    memset (&HostCtl[DEF_USBFS_PORT_INDEX * DEF_ONE_USB_SUP_DEV_TOTAL].InterfaceNum, 0, DEF_ONE_USB_SUP_DEV_TOTAL * sizeof (HOST_CTL));
#endif

    TIM2_Init();
    TIM4_Init();
    GPIO_Config();
    //  InitMouse();

#if DEF_DEBUG_PRINTF
    DUG_PRINTF ("TIM2,4 Delay, GPIO and Mouse Init OK!\r\n");
#endif

    uint8_t data = 0;
    while (1) {


        USART_SendData (USART1, data);
        Delay_Ms (100);
        data++;


        USBH_MainDeal();

        // Handle HID Device
        if (RootHubDev.bType == USB_DEV_CLASS_HID) {

            for (int itf = 0; itf < DEF_INTERFACE_NUM_MAX; itf++) {
                // Handle mouse
                if (HostCtl[0].Interface[itf].HIDRptDesc.type == REPORT_TYPE_MOUSE) {
                    HID_MOUSE_Info_TypeDef *mousemap = USBH_GetMouseInfo (
                        &HostCtl[0].Interface[itf]);
                    ProcessMouse (mousemap);
                }

                // Handle gamepad
                if (HostCtl[0].Interface[itf].HIDRptDesc.type == REPORT_TYPE_JOYSTICK) {

                    HID_gamepad_Info_TypeDef *gamepad = GetGamepadInfo (
                        &HostCtl[0].Interface[itf]);
                    ProcessGamepad (gamepad);
                }

                // Handle Keyboard
                if (HostCtl[0].Interface[itf].HIDRptDesc.type == REPORT_TYPE_KEYBOARD) {
                    // HID_KEYBD_Info_TypeDef *USBH_HID_GetKeybdInfo(Interface *Itf)
                    HID_KEYBD_Info_TypeDef *kbd = USBH_HID_GetKeybdInfo (
                        &HostCtl[0].Interface[itf]);

                    amikb_process (kbd);
                }
            }
        }

        // Handle HUB Device

        if (RootHubDev.bType == USB_DEV_CLASS_HUB) {

            // Iterate over all devices
            for (uint8_t device = 1; device < 5; device++) {
                // Iterate over all interfaces
                for (int itf = 0; itf < DEF_INTERFACE_NUM_MAX; itf++) {
                    // // Handle mouse
                    // if (HostCtl[device].Interface[itf].HIDRptDesc.type == REPORT_TYPE_MOUSE) {
                    //     HID_MOUSE_Info_TypeDef *mousemap = USBH_GetMouseInfo (
                    //         &HostCtl[device].Interface[itf]);
                    //     ProcessMouse (mousemap);
                    // }

                    // // Handle gamepad
                    // if (HostCtl[device].Interface[itf].HIDRptDesc.type == REPORT_TYPE_JOYSTICK) {

                    //  HID_gamepad_Info_TypeDef *gamepad = GetGamepadInfo (
                    //      &HostCtl[device].Interface[itf]);
                    //  ProcessGamepad (gamepad);
                    // }

                    // Handle Keyboard
                    if (HostCtl[device].Interface[itf].HIDRptDesc.type == REPORT_TYPE_KEYBOARD) {
                        HID_KEYBD_Info_TypeDef *kbd = USBH_HID_GetKeybdInfo (
                            &HostCtl[device].Interface[itf]);


                        //  amikb_process (kbd);
                    }
                }
            }
        }
    }
}
