#ifndef UART_UPLOAD_H
#define UART_UPLOAD_H

#include "ch32v00x_conf.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*
     * CH32V003 串口数据上传库（单头文件版）
     *
     * 设计目标：
     * 1. 面向“单片机向上位机上传数据”的场景，默认只启用 USART 发送功能。
     * 2. 所有函数都放在头文件中，直接 include 即可使用，不需要额外添加 .c
     * 文件。
     * 3. 提供文本上传和二进制帧上传两种方式，方便调试和后续上位机解析。
     *
     * 默认硬件连接：
     *   CH32V003 PD5/USART1_TX  ->  USB-TTL 模块 RXD
     *   CH32V003 GND            ->  USB-TTL 模块 GND
     *
     * 注意：
     * - 本开发板的 Type-C 口没有连接到 MCU 的串口 GPIO，只能供电。
     * - 默认不占用 PD6/USART1_RX。如果需要接收上位机命令，可在 include
     * 本文件前 定义 UART_UPLOAD_ENABLE_RX 为 1。
     */

/* 使用的 USART 外设，CH32V003 默认用 USART1。 */
#ifndef UART_UPLOAD_USART
#define UART_UPLOAD_USART USART1
#endif

/* USART 外设时钟。 */
#ifndef UART_UPLOAD_USART_CLK
#define UART_UPLOAD_USART_CLK RCC_APB2Periph_USART1
#endif

/* 串口 TX 所在 GPIO 端口，默认 PD5。 */
#ifndef UART_UPLOAD_TX_GPIO
#define UART_UPLOAD_TX_GPIO GPIOD
#endif

/* 串口 TX 所在 GPIO 端口的时钟。 */
#ifndef UART_UPLOAD_TX_GPIO_CLK
#define UART_UPLOAD_TX_GPIO_CLK RCC_APB2Periph_GPIOD
#endif

/* 串口 TX 引脚，默认 PD5/UTX。 */
#ifndef UART_UPLOAD_TX_PIN
#define UART_UPLOAD_TX_PIN GPIO_Pin_5
#endif

/* TX 引脚输出速度。此模板库支持 2MHz/10MHz/50MHz，默认 50MHz。 */
#ifndef UART_UPLOAD_TX_SPEED
#define UART_UPLOAD_TX_SPEED GPIO_Speed_50MHz
#endif

/*
 * 是否启用 RX 引脚。
 * 0：只发送，适合单片机单向上传数据，节省 PD6。
 * 1：同时初始化 RX，适合后续扩展上位机下发命令。
 */
#ifndef UART_UPLOAD_ENABLE_RX
#define UART_UPLOAD_ENABLE_RX 0
#endif

#if UART_UPLOAD_ENABLE_RX
/* 串口 RX 所在 GPIO 端口，默认 PD6。 */
#ifndef UART_UPLOAD_RX_GPIO
#define UART_UPLOAD_RX_GPIO GPIOD
#endif
/* 串口 RX 所在 GPIO 端口的时钟。 */
#ifndef UART_UPLOAD_RX_GPIO_CLK
#define UART_UPLOAD_RX_GPIO_CLK RCC_APB2Periph_GPIOD
#endif
/* 串口 RX 引脚，默认 PD6/URX。 */
#ifndef UART_UPLOAD_RX_PIN
#define UART_UPLOAD_RX_PIN GPIO_Pin_6
#endif
#endif

/* 二进制帧头第 1 字节，可在 include 前重新定义。 */
#ifndef UART_UPLOAD_FRAME_HEAD0
#define UART_UPLOAD_FRAME_HEAD0 0xA5u
#endif

/* 二进制帧头第 2 字节，可在 include 前重新定义。 */
#ifndef UART_UPLOAD_FRAME_HEAD1
#define UART_UPLOAD_FRAME_HEAD1 0x5Au
#endif

    /*
     * 初始化串口上传功能。
     *
     * 参数：
     *   baudrate：串口波特率，例如 115200。
     *
     * 初始化内容：
     *   1. 开启 GPIO 和 USART 时钟。
     *   2. 配置 TX 为复用推挽输出。
     *   3. 如果 UART_UPLOAD_ENABLE_RX=1，则配置 RX 为浮空输入。
     *   4. 配置串口为 8N1，无硬件流控。
     */
    static inline void
    UART_Upload_Init (uint32_t baudrate)
    {
        GPIO_InitTypeDef gpio = { 0 };
        USART_InitTypeDef usart = { 0 };
        uint32_t gpio_clk = UART_UPLOAD_TX_GPIO_CLK;

#if UART_UPLOAD_ENABLE_RX
        gpio_clk |= UART_UPLOAD_RX_GPIO_CLK;
#endif

        /* GPIO 和 USART 都挂在 APB2，需要先开时钟再配置外设寄存器。 */
        RCC_APB2PeriphClockCmd (gpio_clk | UART_UPLOAD_USART_CLK, ENABLE);

        /* TX 使用复用推挽输出，由 USART 外设驱动引脚电平。 */
        gpio.GPIO_Pin = UART_UPLOAD_TX_PIN;
        gpio.GPIO_Speed = UART_UPLOAD_TX_SPEED;
        gpio.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init (UART_UPLOAD_TX_GPIO, &gpio);

#if UART_UPLOAD_ENABLE_RX
        /* RX 仅在需要接收上位机数据时开启。 */
        gpio.GPIO_Pin = UART_UPLOAD_RX_PIN;
        gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init (UART_UPLOAD_RX_GPIO, &gpio);
#endif

        /* 标准 8N1 串口配置：8 数据位、1 停止位、无校验、无流控。 */
        usart.USART_BaudRate = baudrate;
        usart.USART_WordLength = USART_WordLength_8b;
        usart.USART_StopBits = USART_StopBits_1;
        usart.USART_Parity = USART_Parity_No;
        usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        usart.USART_Mode = USART_Mode_Tx;
#if UART_UPLOAD_ENABLE_RX
        usart.USART_Mode |= USART_Mode_Rx;
#endif

        USART_Init (UART_UPLOAD_USART, &usart);
        USART_Cmd (UART_UPLOAD_USART, ENABLE);
    }

    /*
     * 发送 1 个字节。
     *
     * 这是阻塞式发送：函数会等待发送数据寄存器为空，再写入新字节。
     * 优点是接口简单、占用 RAM 少；缺点是发送期间 CPU 会停在等待循环中。
     */
    static inline void
    UART_Upload_SendByte (uint8_t byte)
    {
        while (USART_GetFlagStatus (UART_UPLOAD_USART, USART_FLAG_TXE)
               == RESET)
            {
            }
        USART_SendData (UART_UPLOAD_USART, byte);
    }

    /*
     * 等待串口最后 1 个字节真正发送完成。
     *
     * TXE 表示发送数据寄存器可写，TC 表示移位寄存器也发送完毕。
     * 如果发送后立刻进入低功耗、复位或切换引脚，建议先调用本函数。
     */
    static inline void
    UART_Upload_Flush (void)
    {
        while (USART_GetFlagStatus (UART_UPLOAD_USART, USART_FLAG_TC) == RESET)
            {
            }
    }

    /*
     * 发送任意内存缓冲区。
     *
     * 参数：
     *   data：待发送数据起始地址。
     *   len ：待发送字节数。
     *
     * 适合发送结构体、数组、采样数据等原始二进制数据。
     */
    static inline void
    UART_Upload_SendBuffer (const void *data, uint16_t len)
    {
        const uint8_t *p = (const uint8_t *)data;

        while (len-- != 0u)
            {
                UART_Upload_SendByte (*p++);
            }
    }

    /*
     * 发送 C 字符串。
     *
     * 字符串必须以 '\0' 结尾。本函数不会自动追加换行。
     */
    static inline void
    UART_Upload_SendString (const char *str)
    {
        while (*str != '\0')
            {
                UART_Upload_SendByte ((uint8_t)*str++);
            }
    }

    /*
     * 发送一行文本。
     *
     * 会在字符串后自动追加 "\r\n"，方便串口助手按行显示。
     */
    static inline void
    UART_Upload_SendLine (const char *str)
    {
        UART_Upload_SendString (str);
        UART_Upload_SendString ("\r\n");
    }

    /*
     * 按小端序发送 16 位无符号整数。
     *
     * 低字节先发，高字节后发。上位机解析二进制数据时需要按同样的
     * little-endian 顺序还原。
     */
    static inline void
    UART_Upload_SendU16LE (uint16_t value)
    {
        UART_Upload_SendByte ((uint8_t)(value & 0xFFu));
        UART_Upload_SendByte ((uint8_t)(value >> 8));
    }

    /*
     * 按小端序发送 32 位无符号整数。
     */
    static inline void
    UART_Upload_SendU32LE (uint32_t value)
    {
        UART_Upload_SendByte ((uint8_t)(value & 0xFFu));
        UART_Upload_SendByte ((uint8_t)((value >> 8) & 0xFFu));
        UART_Upload_SendByte ((uint8_t)((value >> 16) & 0xFFu));
        UART_Upload_SendByte ((uint8_t)(value >> 24));
    }

    /*
     * 以十进制 ASCII 文本发送 32 位无符号整数。
     *
     * 例如 value=1234，串口实际发送字符 '1' '2' '3' '4'。
     * 适合人直接用串口助手查看。
     */
    static inline void
    UART_Upload_SendU32Dec (uint32_t value)
    {
        char buf[10];
        uint8_t i = 0;

        if (value == 0u)
            {
                UART_Upload_SendByte ('0');
                return;
            }

        while (value != 0u)
            {
                buf[i++] = (char)('0' + (value % 10u));
                value /= 10u;
            }

        while (i != 0u)
            {
                UART_Upload_SendByte ((uint8_t)buf[--i]);
            }
    }

    /*
     * 以十进制 ASCII 文本发送 32 位有符号整数。
     *
     * 这里单独处理 INT32_MIN，避免直接对最小负数取反导致有符号溢出。
     */
    static inline void
    UART_Upload_SendI32Dec (int32_t value)
    {
        if (value < 0)
            {
                uint32_t magnitude = (uint32_t)(-(value + 1)) + 1u;
                UART_Upload_SendByte ('-');
                UART_Upload_SendU32Dec (magnitude);
            }
        else
            {
                UART_Upload_SendU32Dec ((uint32_t)value);
            }
    }

    /*
     * 发送无符号整数键值对。
     *
     * 输出格式：
     *   key=value\r\n
     *
     * 示例：
     *   UART_Upload_SendKV_U32("count", 12);
     * 串口输出：
     *   count=12
     */
    static inline void
    UART_Upload_SendKV_U32 (const char *key, uint32_t value)
    {
        UART_Upload_SendString (key);
        UART_Upload_SendByte ('=');
        UART_Upload_SendU32Dec (value);
        UART_Upload_SendString ("\r\n");
    }

    /*
     * 发送有符号整数键值对。
     *
     * 输出格式同 UART_Upload_SendKV_U32。
     */
    static inline void
    UART_Upload_SendKV_I32 (const char *key, int32_t value)
    {
        UART_Upload_SendString (key);
        UART_Upload_SendByte ('=');
        UART_Upload_SendI32Dec (value);
        UART_Upload_SendString ("\r\n");
    }

    /*
     * 发送一帧二进制数据。
     *
     * 帧格式：
     *   head0 head1 type len_l len_h payload checksum
     *
     * 默认展开为：
     *   0xA5 0x5A type len_l len_h payload checksum
     *
     * 字段说明：
     *   head0/head1：固定帧头，用于上位机寻找一帧的开始。
     *   type       ：用户自定义数据类型，例如 0x01 表示传感器数据。
     *   len_l/len_h：payload 长度，小端序，最大 65535 字节。
     *   payload    ：用户数据。
     *   checksum   ：校验和低 8 位。
     *
     * checksum 计算方式：
     *   checksum = (type + len_l + len_h + payload 所有字节之和) & 0xFF
     *
     * 注意：
     * - 本函数不会对 payload 做转义。如果 payload 中出现 0xA5 0x5A，
     *   上位机应依靠 len 字段和 checksum 解析完整帧。
     * - len 使用 uint16_t，适合小数据包上传。CH32V003 RAM 很小，建议单帧
     *   数据保持短小。
     */
    static inline void
    UART_Upload_SendFrame (uint8_t type, const void *payload, uint16_t len)
    {
        const uint8_t *p = (const uint8_t *)payload;
        uint16_t i;
        uint8_t sum = type + (uint8_t)(len & 0xFFu) + (uint8_t)(len >> 8);

        UART_Upload_SendByte (UART_UPLOAD_FRAME_HEAD0);
        UART_Upload_SendByte (UART_UPLOAD_FRAME_HEAD1);
        UART_Upload_SendByte (type);
        UART_Upload_SendU16LE (len);

        for (i = 0; i < len; i++)
            {
                sum = (uint8_t)(sum + p[i]);
                UART_Upload_SendByte (p[i]);
            }

        UART_Upload_SendByte (sum);
    }

#ifdef __cplusplus
}
#endif

#endif
