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
#include "pff_test.h"

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
    Delay_Init();

    DUG_PRINTF ("SystemClk:%d\r\n", SystemCoreClock);
    DUG_PRINTF ("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    DUG_PRINTF ("Bootloader with PFF Library Test Suite\r\n");

    // Initialize IAP subsystem
    IAP_Initialization();

    // Check if GPIO pin is pulled low for full test suite
    // Initialize GPIO for test mode detection
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;      // Use PA0 as test mode pin
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // Input with pull-up
    GPIO_Init (GPIOA, &GPIO_InitStructure);

    uint8_t test_mode = !GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
    uint8_t enumeration_attempts = 0;
    uint8_t enumeration_success = 0;

    if (test_mode) {
        DUG_PRINTF ("Test mode detected (PA0 low) - Will run PFF test suite after USB enumeration...\r\n");
    } else {
        DUG_PRINTF ("Normal mode - Will run IAP after USB enumeration...\r\n");
    }

    // Wait for USB enumeration to complete
    DUG_PRINTF ("Waiting for USB device enumeration...\r\n");
    
    while (enumeration_attempts < 50 && !enumeration_success) {  // Try for ~5 seconds
        // Run one cycle of IAP main deal to advance USB enumeration
        IAP_USBH_PreDeal();
        
        // Check if enumeration completed successfully
        if (IAP_Get_USB_Status(0) == ROOT_DEV_SUCCESS) {
            enumeration_success = 1;
            DUG_PRINTF ("USB enumeration successful!\r\n");
            break;
        }
        
        enumeration_attempts++;
        Delay_Ms(100);
        
        // Show progress every 10 attempts
        if (enumeration_attempts % 10 == 0) {
            DUG_PRINTF ("Enumeration attempt %d/50, status: %d\r\n", enumeration_attempts, IAP_Get_USB_Status(0));
        }
    }

    if (!enumeration_success) {
        DUG_PRINTF ("USB enumeration failed after %d attempts\r\n", enumeration_attempts);
        DUG_PRINTF ("Please check USB device connection\r\n");
        
        if (test_mode) {
            DUG_PRINTF ("Running test suite anyway to show failure modes...\r\n");
            Delay_Ms (1000);
            run_pff_test_suite();
        }
        
        // Blink error indication
        while(1) {
            blinkLed(5, 200);
            Delay_Ms(2000);
        }
    }

    if (test_mode) {
        DUG_PRINTF ("Allowing USB device to stabilize...\r\n");
        Delay_Ms (500);  // Give USB MSC device time to be ready
        
        DUG_PRINTF ("Running PFF quick test...\r\n");
        run_pff_quick_test();
        
        DUG_PRINTF ("Running full PFF test suite...\r\n");
        Delay_Ms (1000);
        run_pff_test_suite();

        DUG_PRINTF ("Tests completed. Remove test jumper and reset to run normal IAP.\r\n");
        while (!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)) {
            blinkLed(1, 100);
            Delay_Ms(2000);
        }
    } else {
        DUG_PRINTF ("Allowing USB device to stabilize...\r\n");
        Delay_Ms (500);  // Give USB MSC device time to be ready
        
        DUG_PRINTF ("Running PFF quick test...\r\n");
        run_pff_quick_test();
        
        DUG_PRINTF ("Starting normal IAP operation...\r\n");
        while (1) {
            IAP_Main_Deal();
        }
    }
}
