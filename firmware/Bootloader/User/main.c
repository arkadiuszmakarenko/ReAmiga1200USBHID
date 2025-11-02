/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/20
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
  USB host IAP demo code.
  USBFS HOST USB drive enumeration and operation, read the APP.BIN file in the root directory inside the USB drive, write to location 0x08005000, after a successful check
  Write the flag data, then jump to the user code. The code will run IAP first to check the flag data, if the preparation data is normal, it will jump to the user code, otherwise
  If the compiled data is OK, it will jump to the user code, if not, it will stay in IAP again.
    Support: FAT12/FAT16/FAT32
*/

#include "usb_host_iap.h"
#include "usb_host_config.h"
#include "utils.h"

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */

int main (void) {

    Delay_Init();
    USART_Printf_Init (115200);
    // Initialize IAP subsystem
    IAP_Initialization();

    // Check if GPIO pin is pulled low for full test suite
    // Initialize GPIO for test mode detection
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init (GPIOA, &GPIO_InitStructure);


    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init (GPIOB, &GPIO_InitStructure);

    if (GPIO_ReadInputDataBit (GPIOA, GPIO_Pin_3) == 0) {

        blinkLed (10, 500);
        while (1) {
            IAP_Main_Deal();
        }
    } else {


        IAP_Jump_APP();
    }
}
