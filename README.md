# CH32V003_Timer_1us

Example Timer project for the CH32V003

Create a timer, TIM2, to increment at a 1us rate, providing a 16-bit timer for micro-second timing

        // USART RX - input pin: D6
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOD, &GPIO_InitStructure);
        // Need BOTH TX and RX:
        USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

Usage example to time printf()

        printf("TIM2: %u\r\n",TIM2->CNT);
        printf("TIM2: %u\r\n",TIM2->CNT); // We should see the timer incrementing
        

Use the timer to measure a function call:

        uint16_t start = TIM2->CNT; // record time a function takes to complete
        function_call();
        uint16_t done = TIM2->CNT; // record time following the function call
        printf("function used %u us\n",(done-start));
        
