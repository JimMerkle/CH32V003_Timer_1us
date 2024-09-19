/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : Jim Merkle
 * Version            : V1.0.0
 * Date               : 2024/08/06
 * Description        : Configure TIM2 for micro-second timing
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/*
 *@Note
  Configure TIM2 to increment every 1us
*/

#include "debug.h"
#include "ch32v00x_tim.h"

/*********************************************************************
 * @fn      TIM2_Init
 *
 * @brief   Initialize TIM2 for 1us continuous increment
 *
 * @return  none
 */
void TIM2_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_TimeBase = {
            .TIM_Prescaler = 47, // 48-1
            .TIM_CounterMode = TIM_CounterMode_Up,
            .TIM_Period = 0xFFFF,
            .TIM_ClockDivision = TIM_CKD_DIV1,
            .TIM_RepetitionCounter = 0x0000};
    TIM_TimeBaseInit(TIM2, &TIM_TimeBase);
    TIM_Cmd(TIM2, ENABLE);
}


/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    Delay_Init();

    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock); // This is just printing a #define value stored in a global

    TIM2_Init(); // initialize TIM2 and start it running

    printf("TIM2: %u\r\n",TIM2->CNT);
    printf("TIM2: %u\r\n",TIM2->CNT); // We should see the timer incrementing

    // Time our Delay_Ms(1) function
    uint16_t start = TIM2->CNT;
    Delay_Ms(1);
    uint16_t done = TIM2->CNT;

    printf("Delay_Ms(1) measured time: %u us\n",done-start);

    while(1); // loop here forever

    return 0;
}
