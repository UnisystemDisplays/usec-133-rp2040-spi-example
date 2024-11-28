#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "usec_dev.h"

/******************************************************************************/

#define SPI_PORT spi0
typedef enum {
  GPIO_MISO = 16,
  GPIO_MOSI = 19,
  GPIO_SCLK = 18,
  GPIO_CS0  = 17,
  GPIO_RST  = 21,
  GPIO_RDY  = 20
} gpio_name;

typedef enum {
  GPIO_HIGH = 1,
  GPIO_LOW  = 0
} gpio_val;

/******************************************************************************/

uint8_t
platform_init                (usec_ctx  *ctx);

uint8_t
platform_deinit              (usec_ctx  *ctx);

uint8_t
platform_delay_ms            (void      *platform,
                              uint16_t   delay_ms);

uint8_t
platform_hw_reset            (void      *platform);

uint8_t
platform_spi_write_byte      (void      *platform,
                              uint8_t    val);

uint8_t
platform_spi_write_bytes     (void      *platform,
                              uint8_t   *data,
                              uint32_t   len);

uint8_t
platform_spi_read_byte       (void      *platform);

uint8_t
platform_spi_cs_high         (void      *platform,
                              uint8_t    cs_num);

uint8_t
platform_spi_cs_low          (void      *platform,
                              uint8_t    cs_num);

uint8_t
platform_gpio_read           (void      *platform);

#endif
