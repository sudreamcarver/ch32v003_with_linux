#include "ch32v00x_conf.h"

/*
 * PD6 最小闪烁测试
 *
 * 这个程序不使用 Delay_Ms、不初始化 USART、不使用 printf。
 * 目的只是验证 main() 是否运行，以及 PD6 是否能被 GPIO 控制。
 */

static void PD6_LED_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_6;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &gpio);
}

static void Raw_Delay(void)
{
    volatile uint32_t i;

    for (i = 0; i < 600000u; i++) {
        __asm volatile("nop");
    }
}

int main(void)
{
    PD6_LED_Init();

    while (1) {
        GPIO_SetBits(GPIOD, GPIO_Pin_6);
        Raw_Delay();
        GPIO_ResetBits(GPIOD, GPIO_Pin_6);
        Raw_Delay();
    }
}
