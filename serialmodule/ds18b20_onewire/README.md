# DS18B20 temperature library

DS18B20 uses the Dallas 1-Wire bus, not hardware I2C. CH32V003 hardware I2C
cannot read a DS18B20 directly.

This folder provides a GPIO 1-Wire DS18B20 driver for the CH32V003 WCH
peripheral library. It supports one device on the bus using `Skip ROM`.

## Wiring

- DS18B20 VDD -> 3.3V
- DS18B20 GND -> GND
- DS18B20 DQ  -> any free GPIO
- Add a 4.7k pull-up resistor from DQ to 3.3V

Avoid PD1 unless you deliberately disable SDI/SWIO, because PD1 is the default
debug pin after reset.

## Example

```c
#include "ds18b20_onewire/ds18b20.h"

static const DS18B20_Bus temp_bus = {
    .gpio = GPIOC,
    .pin = GPIO_Pin_5,
    .gpio_clk = RCC_APB2Periph_GPIOC,
};

int16_t temp_x100;

DS18B20_Init (&temp_bus);
if (DS18B20_ReadTemperatureC_x100 (&temp_bus, &temp_x100) == DS18B20_OK)
    {
        /* temp_x100 = 2537 means 25.37 C. */
    }
```

If your actual sensor is an I2C temperature chip rather than DS18B20, use its
exact model number and I2C register map to write a hardware-I2C driver.
