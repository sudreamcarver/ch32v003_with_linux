#include "uart_upload.h"
#include <debug.h>

/* Configure PD6 as push-pull output for the onboard LED indicator. */
void
LED_PD6_Init (void)
{
    GPIO_InitTypeDef GPIO_InitStructure = { 0 };

    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init (GPIOD, &GPIO_InitStructure);
}

int
main (void)
{
    u8 led_state = 0;
    uint32_t sample_count = 0;

    /* Basic system setup used by the WCH examples. */
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
    SystemCoreClockUpdate ();
    Delay_Init ();

    /*
     * USART1 upload port:
     *   PD5/UTX -> USB-TTL RXD
     *   GND     -> USB-TTL GND
     * Default format is 115200 baud, 8 data bits, no parity, 1 stop bit.
     */
    UART_Upload_Init (115200);

    /* Send boot information as readable key-value text. */
    UART_Upload_SendLine ("CH32V003 UART upload example");
    UART_Upload_SendKV_U32 ("SystemClk", SystemCoreClock);
    UART_Upload_SendKV_U32 ("ChipID", DBGMCU_GetDEVID ());

    LED_PD6_Init ();
    while (1)
        {
            Delay_Ms (500);

            /* Toggle PD6 after each upload so the board shows the loop is running. */
            GPIO_WriteBit (GPIOD, GPIO_Pin_6,
                           (led_state == 0) ? (led_state = Bit_SET)
                                            : (led_state = Bit_RESET));

            sample_count++;
            UART_Upload_SendKV_U32 ("count", sample_count);
            UART_Upload_SendKV_U32 ("pd6", led_state);
        }
}
