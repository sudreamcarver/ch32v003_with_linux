#include "ds18b20.h"
#include "debug.h"

#define DS18B20_CMD_SKIP_ROM 0xCCu
#define DS18B20_CMD_CONVERT_T 0x44u
#define DS18B20_CMD_READ_SCRATCHPAD 0xBEu

static void
ds18b20_drive_low (const DS18B20_Bus *bus)
{
    GPIO_InitTypeDef gpio = { 0 };

    gpio.GPIO_Pin = bus->pin;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (bus->gpio, &gpio);
    GPIO_ResetBits (bus->gpio, bus->pin);
}

static void
ds18b20_release (const DS18B20_Bus *bus)
{
    GPIO_InitTypeDef gpio = { 0 };

    gpio.GPIO_Pin = bus->pin;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (bus->gpio, &gpio);
    GPIO_SetBits (bus->gpio, bus->pin);
}

static uint8_t
ds18b20_read_pin (const DS18B20_Bus *bus)
{
    return GPIO_ReadInputDataBit (bus->gpio, bus->pin) == Bit_SET;
}

static void
ds18b20_write_bit (const DS18B20_Bus *bus, uint8_t bit)
{
    if (bit)
        {
            ds18b20_drive_low (bus);
            Delay_Us (6);
            ds18b20_release (bus);
            Delay_Us (64);
        }
    else
        {
            ds18b20_drive_low (bus);
            Delay_Us (60);
            ds18b20_release (bus);
            Delay_Us (10);
        }
}

static uint8_t
ds18b20_read_bit (const DS18B20_Bus *bus)
{
    uint8_t bit;

    ds18b20_drive_low (bus);
    Delay_Us (6);
    ds18b20_release (bus);
    Delay_Us (9);
    bit = ds18b20_read_pin (bus);
    Delay_Us (55);

    return bit;
}

static void
ds18b20_write_byte (const DS18B20_Bus *bus, uint8_t byte)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
        {
            ds18b20_write_bit (bus, byte & 0x01u);
            byte >>= 1;
        }
}

static uint8_t
ds18b20_read_byte (const DS18B20_Bus *bus)
{
    uint8_t i;
    uint8_t byte = 0;

    for (i = 0; i < 8; i++)
        {
            if (ds18b20_read_bit (bus))
                {
                    byte |= (uint8_t)(1u << i);
                }
        }

    return byte;
}

static uint8_t
ds18b20_crc8 (const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;
    uint8_t i;

    while (len--)
        {
            crc ^= *data++;
            for (i = 0; i < 8; i++)
                {
                    if (crc & 0x01u)
                        {
                            crc = (uint8_t)((crc >> 1) ^ 0x8Cu);
                        }
                    else
                        {
                            crc >>= 1;
                        }
                }
        }

    return crc;
}

void
DS18B20_Init (const DS18B20_Bus *bus)
{
    RCC_APB2PeriphClockCmd (bus->gpio_clk, ENABLE);
    ds18b20_release (bus);
}

uint8_t
DS18B20_Reset (const DS18B20_Bus *bus)
{
    uint8_t presence;

    ds18b20_drive_low (bus);
    Delay_Us (480);
    ds18b20_release (bus);
    Delay_Us (70);
    presence = (uint8_t)!ds18b20_read_pin (bus);
    Delay_Us (410);

    return presence ? DS18B20_OK : DS18B20_ERR_NO_DEVICE;
}

uint8_t
DS18B20_StartConversion (const DS18B20_Bus *bus)
{
    uint16_t timeout_ms = 750;
    uint8_t status;

    status = DS18B20_Reset (bus);
    if (status != DS18B20_OK)
        {
            return status;
        }

    ds18b20_write_byte (bus, DS18B20_CMD_SKIP_ROM);
    ds18b20_write_byte (bus, DS18B20_CMD_CONVERT_T);

    while (!ds18b20_read_bit (bus))
        {
            if (timeout_ms == 0)
                {
                    return DS18B20_ERR_TIMEOUT;
                }
            Delay_Ms (1);
            timeout_ms--;
        }

    return DS18B20_OK;
}

uint8_t
DS18B20_ReadRaw (const DS18B20_Bus *bus, int16_t *raw)
{
    uint8_t scratchpad[9];
    uint8_t i;
    uint8_t status;

    status = DS18B20_Reset (bus);
    if (status != DS18B20_OK)
        {
            return status;
        }

    ds18b20_write_byte (bus, DS18B20_CMD_SKIP_ROM);
    ds18b20_write_byte (bus, DS18B20_CMD_READ_SCRATCHPAD);

    for (i = 0; i < sizeof (scratchpad); i++)
        {
            scratchpad[i] = ds18b20_read_byte (bus);
        }

    if (ds18b20_crc8 (scratchpad, 8) != scratchpad[8])
        {
            return DS18B20_ERR_CRC;
        }

    *raw = (int16_t)((uint16_t)scratchpad[0] | ((uint16_t)scratchpad[1] << 8));
    return DS18B20_OK;
}

uint8_t
DS18B20_ReadTemperatureC_x100 (const DS18B20_Bus *bus, int16_t *temp_x100)
{
    int16_t raw;
    uint8_t status;

    status = DS18B20_StartConversion (bus);
    if (status != DS18B20_OK)
        {
            return status;
        }

    status = DS18B20_ReadRaw (bus, &raw);
    if (status != DS18B20_OK)
        {
            return status;
        }

    *temp_x100 = (int16_t)(((int32_t)raw * 25) / 4);
    return DS18B20_OK;
}
