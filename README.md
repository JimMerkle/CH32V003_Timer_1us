# CH32V003_Timer_1us

Example Timer project for the CH32V003

Create a timer, TIM2, to increment at a 1us rate, providing a 16-bit timer for micro-second timing

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_TimeBase = {
            .TIM_Prescaler = 47, // 48-1
            .TIM_CounterMode = TIM_CounterMode_Up,
            .TIM_Period = 0xFFFF,
            .TIM_ClockDivision = TIM_CKD_DIV1,
            .TIM_RepetitionCounter = 0x0000};
    TIM_TimeBaseInit(TIM2, &TIM_TimeBase);
    TIM_Cmd(TIM2, ENABLE);

Usage example to time printf()

    printf("TIM2: %u\r\n",TIM2->CNT);
    printf("TIM2: %u\r\n",TIM2->CNT); // We should see the timer incrementing
        

Use the timer to measure a function call:

    uint16_t start = TIM2->CNT; // record time a function takes to complete
    function_call();
    uint16_t done = TIM2->CNT; // record time following the function call
    printf("function used %u us\n",(done-start));
        
