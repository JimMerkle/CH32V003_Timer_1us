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
 * @fn      GPIO_Toggle_INIT
 *
 * @brief   Initializes GPIOD.6
 *
 * @return  none
 */
void GPIO_Toggle_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  // initial value
    //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;    // Updated to use PD6
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}

#define I2C_MODE HOST_MODE
/*********************************************************************
 * @fn      IIC_Init
 *
 * @brief   Initializes the IIC peripheral.
 *          Default pin mapping: SCL/PC2, SDA/PC1
 *
 * @return  none
 */
void IIC_Init(u32 bound, u16 address)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    I2C_InitTypeDef I2C_InitStructure={0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_I2C1, ENABLE );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );

    I2C_InitStructure.I2C_ClockSpeed = bound;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitStructure.I2C_OwnAddress1 = address;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init( I2C1, &I2C_InitStructure );

    I2C_Cmd( I2C1, ENABLE );

#if (I2C_MODE == HOST_MODE)
    I2C_AcknowledgeConfig( I2C1, ENABLE );

#endif
}


#define I2C_DS3232_8_BIT (0x68 << 1) // Use 8-bit address value
// Structure to hold I2C status registers along with a us timer value
typedef struct {
    uint16_t time;
    uint16_t status1;
    uint16_t status2;
} I2C_STATUS_INFO;

// It appears our TIM2 is functioning as desired.

I2C_STATUS_INFO i2cstatus[30] = {0}; // clear array
int indx=0; // array index for above

// Capture status registers and time into data structure array
void I2C_read_record(void)
{
    i2cstatus[indx].time = TIM2->CNT;
    i2cstatus[indx].status1 = I2C1->STAR1;
    i2cstatus[indx].status2 = I2C1->STAR2;
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
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock); // This is just printing a #define value stored in a global

    TIM2_Init(); // initialize TIM2 and start it running

    printf("TIM2: %u\r\n",TIM2->CNT);
    printf("TIM2: %u\r\n",TIM2->CNT); // We should see the timer incrementing

    // UART output:
    //SystemClk:48000000
    //TIM2: 0
    //TIM2: 855
    // Printing out "TIM2: 0\r\n" takes 855us, according to the data above.  Does this look correct for 115,200 baud serial?
    // Printing 9 characters, 8 bits / character, 1 start bit, 2 stop bits, no parity = 11 bits / character * 9 characters * 1/115200 = 859us
    // Saleae says 852.5us for this string, 94.75us per character, stop period of 17.23us, bit time of 8.61.
    // Thus, stop is 2 bits long with this hardware, even though 1 stop was requested.

    u8 i = 0;
    GPIO_Toggle_INIT();

    // Clear counter, wait 100 counts, toggle counter, repeat
    for(;;) {
        TIM2->CNT = 0; // clear counter
        while(TIM2->CNT <= 100);
        // Toggle GPIO D6
        GPIO_WriteBit(GPIOD, GPIO_Pin_0, (i == 0) ? (i = Bit_SET) : (i = Bit_RESET)); // Toggle PD6
    } // for-loop


    // Initialize I2C peripheral, Send a 7-bit address (with W/R bit set), watch response due to Not Acknowledged I2C address
    IIC_Init( 80000, 0x02); // 80Kbs, Host I2C address: 0x02 (not used)

    // We wish to follow the "I2C_7bit_Mode" example for the most part....
    while( I2C_GetFlagStatus( I2C1, I2C_FLAG_BUSY ) != RESET ); // spin forever until not busy
    TIM2->CNT = 0; // clear counter/timer
    I2C_read_record(); indx++; // capture status bits

    I2C_GenerateSTART( I2C1, ENABLE ); // Create start condition

    do {
        // capture bit transitions while we wait
        I2C_read_record();
        // if any status bit changed, increment indx
        if()

            i2cstatus[indx].status1 = I2C1->STAR1;
                i2cstatus[indx].status2


    } while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT )); // spin forever until hardware is ready for 7-bit address to be sent

    I2C_Send7bitAddress( I2C1, I2C_DS3232_8_BIT, I2C_Direction_Receiver ); // LSB - R/W bit is set

    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) ); // spin until we can begin reading bytes from device




}
