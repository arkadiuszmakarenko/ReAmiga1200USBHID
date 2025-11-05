/********************************** (C) COPYRIGHT  *******************************
 * File Name          : debug.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/10/24
 * Description        : This file contains all the functions prototypes for UART
 *                      Printf , Delay functions.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef __DEBUG_H
#define __DEBUG_H

#include "stdio.h"
#include "stdarg.h"
#include "stddef.h"
#include "ch32v20x.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Master Debug Control - Set to 0 to completely disable all debug functionality 
 *
 * DEBUG_ENABLE=1: Enables mini_printf functionality (default for development)
 *                 - FLASH usage: ~10.7KB, RAM usage: ~3.9KB
 *                 - Supports: %s, %d, %u, %x, %X, %%
 *
 * DEBUG_ENABLE=0: Completely disables all debug output (production builds)
 *                 - FLASH usage: ~7.9KB, RAM usage: ~3.9KB  
 *                 - All printf/DUG_PRINTF calls become empty macros
 *                 - Saves ~2.9KB FLASH compared to mini_printf
 *                 - Saves ~4.9KB FLASH compared to full printf
 *
 * To disable debug in your build, add: -DDEBUG_ENABLE=0 to compiler flags
 * or change the default value below.
 */
#ifndef DEBUG_ENABLE
#define DEBUG_ENABLE    0
#endif

/* UART Printf Definition */
#define DEBUG_UART1    1
#define DEBUG_UART2    2
#define DEBUG_UART3    3

/* DEBUG UART Definition */
#ifndef DEBUG
#define DEBUG   DEBUG_UART1
#endif

/* SDI Printf Definition */
#define SDI_PR_CLOSE   0
#define SDI_PR_OPEN    1

#ifndef SDI_PRINT
#define SDI_PRINT   SDI_PR_CLOSE
#endif

#if (DEBUG_ENABLE)
/* Debug functions available when DEBUG_ENABLE is set to 1 */
void Delay_Init(void);
void Delay_Us(uint32_t n);
void Delay_Ms(uint32_t n);
void USART_Printf_Init(uint32_t baudrate);
void SDI_Printf_Enable(void);
void mini_printf(const char *format, ...);

  #if(DEBUG)
    #define PRINT(format, ...)    mini_printf(format, ##__VA_ARGS__)
    #define printf(format, ...)   mini_printf(format, ##__VA_ARGS__)
  #else
    #define PRINT(X...)
    #define printf(X...)
  #endif

#else
/* All debug functionality disabled when DEBUG_ENABLE is set to 0 */
void Delay_Init(void);
void Delay_Us(uint32_t n);
void Delay_Ms(uint32_t n);

/* Empty macros - all debug output completely removed */
#define USART_Printf_Init(baudrate)     do { } while(0)
#define SDI_Printf_Enable()             do { } while(0)
#define mini_printf(format, ...)        do { } while(0)
#define PRINT(format, ...)              do { } while(0)
#define printf(format, ...)             do { } while(0)

#endif

#ifdef __cplusplus
}
#endif

#endif
