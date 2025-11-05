/********************************** (C) COPYRIGHT  *******************************
 * File Name          : debug.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : This file contains all the functions prototypes for UART
 *                      Printf , Delay functions.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "debug.h"
#include "stdarg.h"

static uint8_t  p_us = 0;
static uint16_t p_ms = 0;

#if (DEBUG_ENABLE)
#define DEBUG_DATA0_ADDRESS  ((volatile uint32_t*)0xE0000380)
#define DEBUG_DATA1_ADDRESS  ((volatile uint32_t*)0xE0000384)
#endif

/*********************************************************************
 * @fn      Delay_Init
 *
 * @brief   Initializes Delay Funcation.
 *
 * @return  none
 */
void Delay_Init(void)
{
    p_us = SystemCoreClock / 8000000;
    p_ms = (uint16_t)p_us * 1000;
}

/*********************************************************************
 * @fn      Delay_Us
 *
 * @brief   Microsecond Delay Time.
 *
 * @param   n - Microsecond number.
 *
 * @return  None
 */
void Delay_Us(uint32_t n)
{
    uint32_t i;

    SysTick->SR &= ~(1 << 0);
    i = (uint32_t)n * p_us;

    SysTick->CMP = i;
    SysTick->CTLR |= (1 << 4);
    SysTick->CTLR |= (1 << 5) | (1 << 0);

    while((SysTick->SR & (1 << 0)) != (1 << 0));
    SysTick->CTLR &= ~(1 << 0);
}

/*********************************************************************
 * @fn      Delay_Ms
 *
 * @brief   Millisecond Delay Time.
 *
 * @param   n - Millisecond number.
 *
 * @return  None
 */
void Delay_Ms(uint32_t n)
{
    uint32_t i;

    SysTick->SR &= ~(1 << 0);
    i = (uint32_t)n * p_ms;

    SysTick->CMP = i;
    SysTick->CTLR |= (1 << 4);
    SysTick->CTLR |= (1 << 5) | (1 << 0);

    while((SysTick->SR & (1 << 0)) != (1 << 0));
    SysTick->CTLR &= ~(1 << 0);
}

/*********************************************************************
 * @fn      USART_Printf_Init
 *
 * @brief   Initializes the USARTx peripheral.
 *
 * @param   baudrate - USART communication baud rate.
 *
 * @return  None
 */
#if (DEBUG_ENABLE)
void USART_Printf_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

#if(DEBUG == DEBUG_UART1)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

#elif(DEBUG == DEBUG_UART2)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

#elif(DEBUG == DEBUG_UART3)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

#endif

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;

#if(DEBUG == DEBUG_UART1)
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);

#elif(DEBUG == DEBUG_UART2)
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);

#elif(DEBUG == DEBUG_UART3)
    USART_Init(USART3, &USART_InitStructure);
    USART_Cmd(USART3, ENABLE);

#endif
}

/*********************************************************************
 * @fn      SDI_Printf_Enable
 *
 * @brief   Initializes the SDI printf Function.
 *
 * @param   None
 *
 * @return  None
 */
void SDI_Printf_Enable(void)
{
    *(DEBUG_DATA0_ADDRESS) = 0;
    Delay_Init();
    Delay_Ms(1);
}

/*********************************************************************
 * @fn      _write
 *
 * @brief   Support Printf Function
 *
 * @param   *buf - UART send Data.
 *          size - Data length
 *
 * @return  size: Data length
 */
__attribute__((used))
int _write(int fd, char *buf, int size)
{
    int i = 0;

#if (SDI_PRINT == SDI_PR_OPEN)
    int writeSize = size;

    do
    {

        /**
         * data0  data1 8 byte
         * data0 The storage length of the lowest byte, with a maximum of 7 bytes.
         */

        while( (*(DEBUG_DATA0_ADDRESS) != 0u))
        {

        }

        if(writeSize>7)
        {
            *(DEBUG_DATA1_ADDRESS) = (*(buf+i+3)) | (*(buf+i+4)<<8) | (*(buf+i+5)<<16) | (*(buf+i+6)<<24);
            *(DEBUG_DATA0_ADDRESS) = (7u) | (*(buf+i)<<8) | (*(buf+i+1)<<16) | (*(buf+i+2)<<24);

            i += 7;
            writeSize -= 7;
        }
        else
        {
            *(DEBUG_DATA1_ADDRESS) = (*(buf+i+3)) | (*(buf+i+4)<<8) | (*(buf+i+5)<<16) | (*(buf+i+6)<<24);
            *(DEBUG_DATA0_ADDRESS) = (writeSize) | (*(buf+i)<<8) | (*(buf+i+1)<<16) | (*(buf+i+2)<<24);

            writeSize = 0;
        }

    } while (writeSize);


#else
    for(i = 0; i < size; i++){
#if(DEBUG == DEBUG_UART1)
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        USART_SendData(USART1, *buf++);
#elif(DEBUG == DEBUG_UART2)
        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
        USART_SendData(USART2, *buf++);
#elif(DEBUG == DEBUG_UART3)
        while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
        USART_SendData(USART3, *buf++);
#endif
    }
#endif
    return size;
}

/*********************************************************************
 * @fn      mini_putchar
 *
 * @brief   Output single character
 *
 * @param   c - Character to output
 *
 * @return  None
 */
static void mini_putchar(char c)
{
#if (SDI_PRINT == SDI_PR_OPEN)
    while( (*(DEBUG_DATA0_ADDRESS) != 0u));
    *(DEBUG_DATA1_ADDRESS) = 0;
    *(DEBUG_DATA0_ADDRESS) = (1u) | (c << 8);
#else
#if(DEBUG == DEBUG_UART1)
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    USART_SendData(USART1, c);
#elif(DEBUG == DEBUG_UART2)
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
    USART_SendData(USART2, c);
#elif(DEBUG == DEBUG_UART3)
    while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
    USART_SendData(USART3, c);
#endif
#endif
}

/*********************************************************************
 * @fn      mini_puts
 *
 * @brief   Output string
 *
 * @param   s - String to output
 *
 * @return  None
 */
static void mini_puts(const char *s)
{
    while (*s) {
        mini_putchar(*s++);
    }
}

/*********************************************************************
 * @fn      mini_itoa
 *
 * @brief   Convert integer to string with optional zero-padding
 *
 * @param   value - Value to convert
 *          str - Output buffer
 *          base - Base (10 for decimal, 16 for hex)
 *          uppercase - Use uppercase for hex
 *          width - Minimum width (0 for no padding)
 *
 * @return  None
 */
static void mini_itoa(uint32_t value, char *str, int base, int uppercase, int width)
{
    char temp[32];
    int i = 0;
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    
    if (value == 0) {
        temp[i++] = '0';
    } else {
        while (value > 0) {
            temp[i++] = digits[value % base];
            value /= base;
        }
    }
    
    // Add leading zeros if needed
    while (i < width) {
        temp[i++] = '0';
    }
    
    int j = 0;
    while (i > 0) {
        str[j++] = temp[--i];
    }
    str[j] = '\0';
}

/*********************************************************************
 * @fn      mini_printf
 *
 * @brief   Lightweight printf implementation
 *          Supports: %s (string), %d (signed int), %u (unsigned int), 
 *                   %x (lowercase hex), %X (uppercase hex), 
 *                   %08X (zero-padded 8-digit uppercase hex), %% (literal %)
 *
 * @param   format - Format string
 *          ... - Arguments
 *
 * @return  None
 */
void mini_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    const char *p = format;
    char buffer[32];
    
    while (*p) {
        if (*p == '%' && *(p + 1)) {
            p++; // Skip '%'
            
            // Check for width specifier (e.g., 08)
            int width = 0;
            int zero_pad = 0;
            if (*p == '0') {
                zero_pad = 1;
                p++;
            }
            while (*p >= '0' && *p <= '9') {
                width = width * 10 + (*p - '0');
                p++;
            }
            
            switch (*p) {
                case 's': {
                    const char *s = va_arg(args, const char*);
                    if (s) {
                        mini_puts(s);
                    } else {
                        mini_puts("(null)");
                    }
                    break;
                }
                case 'd': {
                    int value = va_arg(args, int);
                    if (value < 0) {
                        mini_putchar('-');
                        value = -value;
                    }
                    mini_itoa((uint32_t)value, buffer, 10, 0, zero_pad ? width : 0);
                    mini_puts(buffer);
                    break;
                }
                case 'u': {
                    uint32_t value = va_arg(args, uint32_t);
                    mini_itoa(value, buffer, 10, 0, zero_pad ? width : 0);
                    mini_puts(buffer);
                    break;
                }
                case 'x': {
                    uint32_t value = va_arg(args, uint32_t);
                    mini_itoa(value, buffer, 16, 0, zero_pad ? width : 0);
                    mini_puts(buffer);
                    break;
                }
                case 'X': {
                    uint32_t value = va_arg(args, uint32_t);
                    mini_itoa(value, buffer, 16, 1, zero_pad ? width : 0);
                    mini_puts(buffer);
                    break;
                }
                case '%': {
                    mini_putchar('%');
                    break;
                }
                default: {
                    // Unknown format, just output as-is
                    mini_putchar('%');
                    mini_putchar(*p);
                    break;
                }
            }
        } else {
            mini_putchar(*p);
        }
        p++;
    }
    
    va_end(args);
}

#endif /* DEBUG_ENABLE */

/*********************************************************************
 * @fn      _write
 *
 * @brief   Support Printf Function (stub when DEBUG_ENABLE is 0)
 *
 * @param   *buf - UART send Data.
 *          size - Data length
 *
 * @return  size: Data length
 */
#if (!DEBUG_ENABLE)
__attribute__((used))
int _write(int fd, char *buf, int size)
{
    // Return size to avoid linker errors, but do nothing
    return size;
}
#endif

/*********************************************************************
 * @fn      _sbrk
 *
 * @brief   Change the spatial position of data segment.
 *
 * @return  size: Data length
 */
__attribute__((used))
void *_sbrk(ptrdiff_t incr)
{
    extern char _end[];
    extern char _heap_end[];
    static char *curbrk = _end;

    if ((curbrk + incr < _end) || (curbrk + incr > _heap_end))
    return NULL - 1;

    curbrk += incr;
    return curbrk - incr;
}
