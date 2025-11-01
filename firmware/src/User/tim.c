#include "tim.h"
#include "mouse.h"

void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));


void TIM2_Init( void )
{


    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = { 0 };
    NVIC_InitTypeDef NVIC_InitStructure = { 0 };

    /* Enable Timer3 Clock */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );

    /* Initialize Timer2 */
    TIM_TimeBaseStructure.TIM_Period = 1000;
    TIM_TimeBaseStructure.TIM_Prescaler = 18720-1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit( TIM2, &TIM_TimeBaseStructure );

    TIM_ITConfig( TIM2, TIM_IT_Update, ENABLE );

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );

    /* Enable Timer2 */
    TIM_Cmd( TIM2, ENABLE );

    /* Enable timer2 interrupt */
    NVIC_EnableIRQ( TIM2_IRQn );

}

void TIM4_Init( void )
{


    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = { 0 };
    NVIC_InitTypeDef NVIC_InitStructure = { 0 };

    /* Enable Timer4 Clock */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM4, ENABLE );

    /* Initialize Timer4 */
    TIM_TimeBaseStructure.TIM_Period = 1000;
    TIM_TimeBaseStructure.TIM_Prescaler = 18720-1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit( TIM4, &TIM_TimeBaseStructure );

    TIM_ITConfig( TIM4, TIM_IT_Update, ENABLE );

    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );

    /* Enable Timer4 */
    TIM_Cmd( TIM4, ENABLE );

    /* Enable timer4 interrupt */
    NVIC_EnableIRQ( TIM4_IRQn );

}



void TIM2_IRQHandler( void )
{

    if( TIM_GetITStatus( TIM2, TIM_IT_Update ) != RESET )
    {
    	ProcessX_IRQ();
        /* Clear interrupt flag */
        TIM_ClearITPendingBit( TIM2, TIM_IT_Update );
    }
}

void TIM4_IRQHandler( void )
{

    if( TIM_GetITStatus( TIM4, TIM_IT_Update ) != RESET )
    {
    	 ProcessY_IRQ();
        /* Clear interrupt flag */
        TIM_ClearITPendingBit( TIM4, TIM_IT_Update );
    }
}
