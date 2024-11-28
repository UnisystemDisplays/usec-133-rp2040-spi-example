#include <string.h>
#include "platform.h"

/*
 * platform_init()
 */
uint8_t
platform_init (usec_ctx *ctx)
{
  /* spi init */
  spi_init (SPI_PORT, 25*1000*1000);
  gpio_set_function (GPIO_MISO, GPIO_FUNC_SPI);
  gpio_set_function (GPIO_CS0,  GPIO_FUNC_SIO);
  gpio_set_function (GPIO_SCLK, GPIO_FUNC_SPI);
  gpio_set_function (GPIO_MOSI, GPIO_FUNC_SPI);

  /* init gpio direction */
  gpio_set_dir (GPIO_RDY, GPIO_IN);
  gpio_set_dir (GPIO_RST, GPIO_OUT);
  gpio_set_dir (GPIO_CS0, GPIO_OUT);

  /* set default gpio values */
  gpio_put (GPIO_RST, GPIO_HIGH);
  gpio_put (GPIO_CS0, GPIO_HIGH);

  /* init platform specific data - not used */
  ctx->platform = NULL;

  return USEC_DEV_OK;
}

/*
 * platform_deinit()
 */
uint8_t
platform_deinit (usec_ctx *ctx)
{
  spi_deinit (SPI_PORT);
  return USEC_DEV_OK;
}

/*
 * platform_delay_ms()
 */
uint8_t
platform_delay_ms (void      *platform,
                   uint16_t   delay_ms)
{
  sleep_ms (delay_ms);

  return USEC_DEV_OK;
}

/*
 * platform_hw_reset()
 */
uint8_t
platform_hw_reset (void *platform)
{
  gpio_put (GPIO_RST, GPIO_LOW);
  platform_delay_ms (platform, 100);
  gpio_put (GPIO_RST, GPIO_HIGH);
  platform_delay_ms (platform, 200);

  return USEC_DEV_OK;
}

/*
 * platform_spi_write_byte()
 */
uint8_t
platform_spi_write_byte (void    *platform,
                         uint8_t  val)
{
  spi_write_blocking (SPI_PORT, &val, 1);

  return USEC_DEV_OK;
}

/*
 * platform_spi_write_bytes()
 */
uint8_t
platform_spi_write_bytes (void      *platform,
                          uint8_t   *data,
                          uint32_t   len)
{
  spi_write_blocking (SPI_PORT, data, len);

  return USEC_DEV_OK;
}

/*
 * platform_spi_read_byte()
 */
uint8_t
platform_spi_read_byte (void *platform)
{
  uint8_t val;

  spi_read_blocking (SPI_PORT, 0x00, &val, 1);

  return val;
}

/*
 * platform_spi_cs_high()
 */
uint8_t
platform_spi_cs_high (void     *platform,
                      uint8_t   cs_num)
{
  gpio_put (GPIO_CS0, GPIO_HIGH);
  return USEC_DEV_OK;
}

/*
 * platform_spi_cs_low()
 */
uint8_t
platform_spi_cs_low (void     *platform,
                     uint8_t   cs_num)
{
  gpio_put (GPIO_CS0, GPIO_LOW);
  return USEC_DEV_OK;
}

/*
 * platform_gpio_read()
 */
uint8_t
platform_gpio_read (void *platform)
{
  return gpio_get (GPIO_RDY);
}
