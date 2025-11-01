#include "utils.h"

void blinkLed(int number,int delayms)
{
    int i;

    for (i = 0; i < number; i++) {
        GPIO_WriteBit(GPIOC, GPIO_Pin_14, Bit_RESET);
        Delay_Ms(delayms);
        GPIO_WriteBit(GPIOC, GPIO_Pin_14, Bit_SET);
        Delay_Ms(delayms);
    }
}

void InitStuff(void)
{
        GPIO_InitTypeDef GPIO_InitStructure;



        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);


        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        GPIO_WriteBit(GPIOC, GPIO_Pin_14, Bit_SET);


}


void DeInitStuff(void)
{
    GPIO_DeInit(GPIOA);
    //GPIO_DeInit(GPIOC);
   // USBFS_Host_Init( DISABLE );

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, DISABLE);

}


