#ifndef __TIM_H
#define __TIM_H

#include "usb_host_config.h"

void TIM2_Init( void );
void TIM4_Init( void );


void TIM2_IRQHandler( void );
void TIM4_IRQHandler( void );

#endif /*__CH32V10x_SYSTEM_H */
