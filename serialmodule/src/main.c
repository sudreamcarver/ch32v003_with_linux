#include "uart_upload.h"
#include <debug.h>

/* Configure PD0 as push-pull output for the onboard LED example. */
void
GPIO_Toggle (void)
{
    GPIO_InitTypeDef GPIO_InitStructure = { 0 };

    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init (GPIOD, &GPIO_InitStructure);
}

int
main (void)
{
    u8 led = 0;
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

    GPIO_Toggle ();
    while (1)
        {
            uint16_t payload[2];

            Delay_Ms (500);

            /* Toggle PD0 every 500 ms and upload the current state. */
            GPIO_WriteBit (GPIOD, GPIO_Pin_0,
                           (led == 0) ? (led = Bit_SET) : (led = Bit_RESET));

            sample_count++;
            UART_Upload_SendKV_U32 ("count", sample_count);
            UART_Upload_SendKV_U32 ("led", led);

            /* Also upload the same data as a compact binary frame, type 0x01.
             */
            payload[0] = (uint16_t)sample_count;
            payload[1] = (uint16_t)led;
            UART_Upload_SendFrame (0x01, payload, sizeof (payload));
        }
}
