#ifndef DS18B20_H
#define DS18B20_H

#include "ch32v00x_conf.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    GPIO_TypeDef *gpio;
    uint16_t pin;
    uint32_t gpio_clk;
} DS18B20_Bus;

#define DS18B20_OK 0u
#define DS18B20_ERR_NO_DEVICE 1u
#define DS18B20_ERR_TIMEOUT 2u
#define DS18B20_ERR_CRC 3u

void DS18B20_Init (const DS18B20_Bus *bus);
uint8_t DS18B20_Reset (const DS18B20_Bus *bus);
uint8_t DS18B20_StartConversion (const DS18B20_Bus *bus);
uint8_t DS18B20_ReadRaw (const DS18B20_Bus *bus, int16_t *raw);
uint8_t DS18B20_ReadTemperatureC_x100 (const DS18B20_Bus *bus,
                                       int16_t *temp_x100);

#ifdef __cplusplus
}
#endif

#endif
