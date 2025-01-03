#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "usec_dev.h"
#include "platform.h"

/******************************************************************************/

/*
 * it8951 - commands
 */

#define IT8951_TCON_SYS_RUN      (0x0001)
#define IT8951_TCON_STANDBY      (0x0002)
#define IT8951_TCON_SLEEP        (0x0003)
#define IT8951_TCON_REG_RD       (0x0010)
#define IT8951_TCON_REG_WR       (0x0011)
#define IT8951_TCON_MEM_BST_RD_T (0x0012)
#define IT8951_TCON_MEM_BST_RD_S (0x0013)
#define IT8951_TCON_MEM_BST_WR   (0x0014)
#define IT8951_TCON_MEM_BST_END  (0x0015)
#define IT8951_TCON_LD_IMG       (0x0020)
#define IT8951_TCON_LD_IMG_AREA  (0x0021)
#define IT8951_TCON_LD_IMG_END   (0x0022)
#define IT8951_CMD_GET_DEV_INFO  (0x0302)
#define IT8951_CMD_DPY_AREA      (0x0034)
#define IT8951_CMD_DPY_BUF_AREA  (0x0037)
#define IT8951_CMD_PWR_SEQUENCE  (0x0038)
#define IT8951_CMD_VCOM          (0x0039)
#define IT8951_CMD_TEMP          (0x0040)

/*
 * it8951 - selected registers
 */

#define IT8951_SYST_REG_BASE     0x0000
#define IT8951_DISP_REG_BASE     0x1000
#define IT8951_MCSR_BASE_ADDR    0x0200
#define IT8951_I80CPCR           (IT8951_SYST_REG_BASE + 0x04)
#define IT8951_LUT0EWHR          (IT8951_DISP_REG_BASE + 0x00)
#define IT8951_LUT0XYR           (IT8951_DISP_REG_BASE + 0x40)
#define IT8951_LUT0BADDR         (IT8951_DISP_REG_BASE + 0x80)
#define IT8951_LUT0MFN           (IT8951_DISP_REG_BASE + 0xC0)
#define IT8951_LUT01AF           (IT8951_DISP_REG_BASE + 0x114)
#define IT8951_UP0SR             (IT8951_DISP_REG_BASE + 0x134)
#define IT8951_UP1SR             (IT8951_DISP_REG_BASE + 0x138)
#define IT8951_LUT0ABFRV         (IT8951_DISP_REG_BASE + 0x13C)
#define IT8951_UPBBADDR          (IT8951_DISP_REG_BASE + 0x17C)
#define IT8951_LUT0IMXY          (IT8951_DISP_REG_BASE + 0x180)
#define IT8951_LUTAFSR           (IT8951_DISP_REG_BASE + 0x224)
#define IT8951_BGVR              (IT8951_DISP_REG_BASE + 0x250)
#define IT8951_MCSR              (IT8951_MCSR_BASE_ADDR + 0x0000)
#define IT8951_LISAR             (IT8951_MCSR_BASE_ADDR + 0x0008)

/*
 * it8951 - utils
 */

#define IT8951_ROTATE_0          0
#define IT8951_ROTATE_90         1
#define IT8951_ROTATE_180        2
#define IT8951_ROTATE_270        3

#define IT8951_2BPP              0
#define IT8951_3BPP              1
#define IT8951_4BPP              2
#define IT8951_8BPP              3

#define IT8951_LDIMG_L_ENDIAN    0
#define IT8951_LDIMG_B_ENDIAN    1

/******************************************************************************/

typedef struct
{
  uint16_t width;
  uint16_t height;
  uint16_t mem_addr_l;
  uint16_t mem_addr_h;
  uint16_t fw_ver[8];
  uint16_t lut_ver[8];
} it8951_sys_info;

typedef struct
{
  uint8_t   *source_data;
  uint32_t   memory_addr;
  uint16_t   endian_type;
  uint16_t   img_pixel_format;
  uint16_t   img_rotate;
} it8951_load_img_info;

typedef struct
{
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
} it8951_area_img_info;

/******************************************************************************/

/*
 * it8951_wait_busy()
 */
static void
it8951_wait_busy (usec_ctx *ctx)
{
  while (platform_gpio_read (ctx->platform) == 0);
}

/*
 * it8951_write_command()
 */
static uint8_t
it8951_write_command (usec_ctx  *ctx,
                      uint8_t    id,
                      uint16_t   command)
{
  uint16_t write_preamble;
  uint8_t status;

  write_preamble = 0x6000;

  it8951_wait_busy (ctx);

  status = platform_spi_cs_low (ctx->platform, id);

  status |= platform_spi_write_byte (ctx->platform, write_preamble >> 8);
  status |= platform_spi_write_byte (ctx->platform, write_preamble);

  it8951_wait_busy (ctx);

  status |= platform_spi_write_byte (ctx->platform, command >> 8);
  status |= platform_spi_write_byte (ctx->platform, command);

  status |= platform_spi_cs_high (ctx->platform, id);

  return status;
}

/*
 * it8951_write_data()
 */
static uint8_t
it8951_write_data (usec_ctx  *ctx,
                   uint8_t    id,
                   uint16_t   data)
{
  uint16_t write_preamble;
  uint8_t status;

  write_preamble = 0x0000;

  it8951_wait_busy (ctx);

  status = platform_spi_cs_low (ctx->platform, id);

  status |= platform_spi_write_byte (ctx->platform, write_preamble >> 8);
  status |= platform_spi_write_byte (ctx->platform, write_preamble);

  it8951_wait_busy (ctx);

  status |= platform_spi_write_byte (ctx->platform, data >> 8);
  status |= platform_spi_write_byte (ctx->platform, data);

  status |= platform_spi_cs_high (ctx->platform, id);

  return status;
}

/*
 * it8951_write_multi_data()
 */
static uint8_t
it8951_write_multi_data (usec_ctx  *ctx,
                         uint8_t    id,
                         uint8_t   *data,
                         uint32_t   len)
{
  uint16_t write_preamble;
  uint8_t status;

  write_preamble = 0x0000;

  it8951_wait_busy (ctx);

  status = platform_spi_cs_low (ctx->platform, id);

  status |= platform_spi_write_byte (ctx->platform, write_preamble >> 8);
  status |= platform_spi_write_byte (ctx->platform, write_preamble);

  it8951_wait_busy (ctx);

  status |= platform_spi_write_bytes (ctx->platform, data, len);

  status |= platform_spi_cs_high (ctx->platform, id);

  return status;
}

/*
 * it8951_read_data()
 */
static uint16_t
it8951_read_data (usec_ctx  *ctx,
                  uint8_t    id)
{
  uint16_t write_preamble = 0x1000;
  uint16_t read_data;
  uint16_t read_dummy;

  it8951_wait_busy (ctx);

  platform_spi_cs_low (ctx->platform, id);

  platform_spi_write_byte (ctx->platform, write_preamble >> 8);
  platform_spi_write_byte (ctx->platform, write_preamble);

  it8951_wait_busy (ctx);

  read_dummy = platform_spi_read_byte (ctx->platform) << 8;
  read_dummy |= platform_spi_read_byte (ctx->platform);

  it8951_wait_busy (ctx);

  read_data = platform_spi_read_byte (ctx->platform) << 8;
  read_data |= platform_spi_read_byte (ctx->platform);

  platform_spi_cs_high (ctx->platform, id);

  return read_data;
}

/*
 * it8951_read_multi_data()
 */
static uint8_t
it8951_read_multi_data (usec_ctx  *ctx,
                        uint8_t    id,
                        uint16_t  *data,
                        uint32_t   len)
{
  uint16_t write_preamble = 0x1000;
  uint16_t read_dummy;
  uint8_t status;

  it8951_wait_busy (ctx);

  status = platform_spi_cs_low (ctx->platform, id);

  status |= platform_spi_write_byte (ctx->platform, write_preamble >> 8);
  status |= platform_spi_write_byte (ctx->platform, write_preamble);

  it8951_wait_busy (ctx);

  read_dummy = platform_spi_read_byte (ctx->platform) << 8;
  read_dummy |= platform_spi_read_byte (ctx->platform);

  it8951_wait_busy (ctx);

  for (uint32_t i = 0; i < len; i++)
    {
      data[i] = platform_spi_read_byte (ctx->platform) << 8;
      data[i] |= platform_spi_read_byte (ctx->platform);
    }

  status |= platform_spi_cs_high (ctx->platform, id);

  return status;
}

/*
 * it8951_write_multi_args()
 */
static uint8_t
it8951_write_multi_args (usec_ctx  *ctx,
                         uint8_t    id,
                         uint16_t   cmd,
                         uint16_t  *arg,
                         uint16_t   num)
{
  uint8_t status;

  status = it8951_write_command (ctx, id, cmd);

  for (uint16_t i = 0; i < num; i++)
    status |= it8951_write_data (ctx, id, arg[i]);

  return status;
}

/*
 * it8951_read_reg()
 */
static uint16_t
it8951_read_reg (usec_ctx  *ctx,
                 uint8_t    id,
                 uint16_t   reg)
{
  it8951_write_command (ctx, id, IT8951_TCON_REG_RD);
  it8951_write_data (ctx, id, reg);

  return it8951_read_data (ctx, id);
}

/*
 * it8951_write_reg()
 */
static uint8_t
it8951_write_reg (usec_ctx  *ctx,
                  uint8_t    id,
                  uint16_t   reg,
                  uint16_t   val)
{
  uint8_t status;

  status = it8951_write_command (ctx, id, IT8951_TCON_REG_WR);
  status |= it8951_write_data (ctx, id, reg);
  status |= it8951_write_data (ctx, id, val);

  return status;
}

/*
 * it8951_get_system_info()
 */
static uint8_t
it8951_get_system_info (usec_ctx         *ctx,
                        uint8_t           id,
                        it8951_sys_info  *info)
{
  uint8_t status;

  status = it8951_write_command (ctx, id, IT8951_CMD_GET_DEV_INFO);
  status |= it8951_read_multi_data (ctx, id, (uint16_t *)info,
                                    sizeof (it8951_sys_info) / 2);

  return status;
}

/*
 * it8951_get_temp()
 */
static uint8_t
it8951_get_temp (usec_ctx  *ctx,
                 uint8_t    id,
                 uint8_t   *temp)
{
  uint16_t val[2];
  uint8_t status;

  status = it8951_write_command (ctx, id, IT8951_CMD_TEMP);
  status |= it8951_write_data (ctx, id, 0x0000);

  platform_delay_ms (ctx->platform, 50);

  it8951_read_multi_data (ctx, id, val, 2);
  *temp = (int8_t) (val[0] & 0xFF);

  return status;
}

/*
 * it8951_set_target_mem_addr()
 */
static uint8_t
it8951_set_target_mem_addr (usec_ctx  *ctx,
                            uint8_t    id,
                            uint32_t   addr)
{
  uint16_t addr_h;
  uint16_t addr_l;
  uint8_t status;

  addr_h = (uint16_t) ((addr >> 16) & 0x0000FFFF);
  addr_l = (uint16_t) (addr & 0x0000FFFF);

  status = it8951_write_reg (ctx, id, IT8951_LISAR + 2, addr_h);
  status |= it8951_write_reg (ctx, id, IT8951_LISAR, addr_l);

  return status;
}

/*
 * it8951_wait_display_ready()
 */
static void
it8951_wait_display_ready (usec_ctx  *ctx,
                           uint8_t    id)
{
  while (it8951_read_reg (ctx, id, IT8951_LUTAFSR));
}

/*
 * it8951_load_img_area_start()
 */
static uint8_t
it8951_load_img_area_start (usec_ctx              *ctx,
                            uint8_t                id,
                            it8951_load_img_info  *load_img_info,
                            it8951_area_img_info  *area_img_info)
{
  uint16_t args[5];

  args[0] = (load_img_info->endian_type      << 8 |
             load_img_info->img_pixel_format << 4 |
             load_img_info->img_rotate);

  args[1] = area_img_info->x;
  args[2] = area_img_info->y;
  args[3] = area_img_info->w;
  args[4] = area_img_info->h;

  return it8951_write_multi_args (ctx, id, IT8951_TCON_LD_IMG_AREA, args, 5);
}

/*
 * it8951_load_img_end()
 */
static uint8_t
it8951_load_img_end (usec_ctx  *ctx,
                     uint8_t    id)
{
  return it8951_write_command (ctx, id, IT8951_TCON_LD_IMG_END);
}

/*
 * it8951_packed_pixel_write_1bpp()
 */
static uint8_t
it8951_packed_pixel_write_1bpp (usec_ctx              *ctx,
                                uint8_t                id,
                                it8951_load_img_info  *load_img_info,
                                it8951_area_img_info  *area_img_info)
{
  uint8_t   status;

  status = it8951_set_target_mem_addr (ctx, id, load_img_info->memory_addr);
  status |= it8951_load_img_area_start (ctx, id, load_img_info, area_img_info);

  status |= it8951_write_multi_data (ctx, id,
                                     load_img_info->source_data,
                                     area_img_info->w * area_img_info->h);

  status |= it8951_load_img_end (ctx, id);
  return status;
}

/*
 * it8951_packed_pixel_write_8bpp()
 */
static uint8_t
it8951_packed_pixel_write_8bpp (usec_ctx              *ctx,
                                uint8_t                id,
                                it8951_load_img_info  *load_img_info,
                                it8951_area_img_info  *area_img_info)
{
  uint8_t   status;

  status = it8951_set_target_mem_addr (ctx, id, load_img_info->memory_addr);
  status |= it8951_load_img_area_start (ctx, id, load_img_info, area_img_info);

  status |= it8951_write_multi_data (ctx, id,
                                     load_img_info->source_data,
                                     area_img_info->w * area_img_info->h);

  status |= it8951_load_img_end (ctx, id);
  return status;
}

/*
 * it8951_display_area()
 */
static uint8_t
it8951_display_area (usec_ctx  *ctx,
                     uint8_t    id,
                     uint16_t   x,
                     uint16_t   y,
                     uint16_t   w,
                     uint16_t   h,
                     uint16_t   mode)
{
  uint16_t args[5];

  args[0] = x;
  args[1] = y;
  args[2] = w;
  args[3] = h;
  args[4] = mode;

  return it8951_write_multi_args (ctx, id, IT8951_CMD_DPY_AREA, args, 5);
}

/*
 * it8951_pmic_power_off()
 */
static uint8_t
it8951_pmic_power_off (usec_ctx  *ctx,
                       uint8_t    id)
{
  uint8_t status;

  status = it8951_write_command (ctx, id, IT8951_CMD_PWR_SEQUENCE);
  status |= it8951_write_data (ctx, id, 0x0000);

  return status;
}

/******************************************************************************/

/*
 * usec_dev_log()
 */
static void
usec_dev_log (const char* fmt, ...)
{
#if USEC_DEV_DEBUG_LOG
  va_list args;

  va_start (args, fmt);
  vprintf (fmt, args);
  va_end (args);
#endif
}

/******************************************************************************/

/*
 * usec_init()
 */
usec_ctx *
usec_init (void)
{
  usec_ctx *ctx;
  it8951_sys_info info;
  uint8_t status;

  /* init usec context */
  ctx = malloc(sizeof(*ctx));
  if (ctx == NULL)
    {
      usec_dev_log ("[usec] error: cannot initialize device context\n\r");
      return NULL;
    }

  /* init low-level hardware */
  status = platform_init (ctx);
  if (status != USEC_DEV_OK)
    {
      usec_dev_log ("[usec] error: cannot initialize host platform\n\r");

      free (ctx);
      return NULL;
    }

  /* reset controller */
  status = platform_hw_reset (ctx->platform);
  if (status != USEC_DEV_OK)
    {
      usec_dev_log ("[usec] error: cannot reset controller\n\r");

      platform_deinit (ctx);
      free (ctx);
      return NULL;
    }

  /* get system info */
  status = it8951_get_system_info (ctx, 0, &info);
  if (status != USEC_DEV_OK)
    {
      usec_dev_log ("[usec] error: cannot read data from controller\n\r");

      platform_deinit (ctx);
      free (ctx);
      return NULL;
    }

  /* enable pack write mode */
  status = it8951_write_reg (ctx, 0, IT8951_I80CPCR, 0x0001);
  if (ctx == NULL)
    {
      usec_dev_log ("[usec] error: cannot enable pack-write mode\n\r");

      platform_deinit (ctx);
      free (ctx);
      return NULL;
    }

  ctx->dev_addr[0] = info.mem_addr_l | (info.mem_addr_h << 16);
  ctx->dev_width[0] = info.width;
  ctx->dev_height[0] = info.height;

  usec_dev_log ("[usec] status: screen width - %d [px]\n\r",
                ctx->dev_width[0]);
  usec_dev_log ("[usec] status: screen height - %d [px]\n\r",
                ctx->dev_height[0]);

  return ctx;
}

/*
 * usec_deinit()
 */
void
usec_deinit (usec_ctx *ctx)
{
  if (ctx == NULL)
    {
      usec_dev_log ("[usec] error: invalid device context\n\r");
      return;
    }

  platform_deinit (ctx);
  free (ctx);
}

/*
 * usec_get_temp()
 */
uint8_t
usec_get_temp (usec_ctx  *ctx,
               uint8_t   *temp_val)
{
  uint8_t status;

  if (ctx == NULL)
    {
      usec_dev_log ("[usec] error: invalid device context\n\r");
      return USEC_DEV_ERR;
    }

  status = it8951_get_temp (ctx, 0, temp_val);
  if (status == USEC_DEV_OK)
    usec_dev_log ("[usec] status: screen temp - %d [degC]\n\r", *temp_val);
  else
    usec_dev_log ("[usec] error: cannot read data from temperature sensor\n\r");

  return status;
}

/*
 * usec_1bpp_mode()
 */
uint8_t
usec_1bpp_mode (usec_ctx  *ctx,
                uint8_t    enable)
{
  uint8_t reg;
  uint8_t status;

  if (ctx == NULL)
    {
      usec_dev_log ("[usec] error: invalid device context\n\r");
      return USEC_DEV_ERR;
    }

  if (enable)
    {
      reg = it8951_read_reg (ctx, 0, IT8951_UP1SR + 2);

      status  = it8951_write_reg (ctx, 0, IT8951_UP1SR + 2, reg | (1 << 2));
      status |= it8951_write_reg (ctx, 0, IT8951_BGVR, (0x00 << 8) | 0xF0);
    }
  else
    {
      reg = it8951_read_reg (ctx, 0, IT8951_UP1SR + 2);
      status = it8951_write_reg (ctx, 0, IT8951_UP1SR + 2, reg & ~(1 << 2));
    }

  return status;
}

/*
 * usec_img_upload()
 */
uint8_t
usec_img_upload (usec_ctx  *ctx,
                 uint8_t   *img_data,
                 size_t     img_size,
                 uint8_t    img_bpp,
                 uint32_t   img_pos_x,
                 uint32_t   img_pos_y,
                 uint32_t   img_width,
                 uint32_t   img_height)
{
  it8951_load_img_info load_img_info;
  it8951_area_img_info area_img_info;
  uint8_t status;

  status = USEC_DEV_ERR;

  if (ctx == NULL)
    {
      usec_dev_log ("[usec] error: invalid device context\n\r");
      return USEC_DEV_ERR;
    }

  if ((img_bpp != IMG_1BPP) && (img_bpp != IMG_8BPP))
    {
      usec_dev_log ("[usec] error: library supports only IMG_1BPP " \
                    "and IMG_8BPP modes\n\r");
      return USEC_DEV_ERR;
    }

  if ((img_width == 0) || (img_height == 0))
    {
      usec_dev_log ("[usec] error: invalid image data width/height\n\r");
      return USEC_DEV_ERR;
    }

  if (((img_bpp == IMG_1BPP) && (img_width * img_height) != (img_size * 8)) ||
      ((img_bpp == IMG_8BPP) && (img_width * img_height) != (img_size)))
    {
      usec_dev_log ("[usec] error: invalid image data size\n\r");
      return USEC_DEV_ERR;
    }

  if ((img_pos_x + img_width > ctx->dev_width[0]) || \
      (img_pos_y + img_height > ctx->dev_height[0]))
    {
      usec_dev_log ("[usec] error: image out of screen bounds\n\r");
      return USEC_DEV_ERR;
    }

  it8951_wait_display_ready (ctx, 0);

  load_img_info.source_data      = img_data;
  load_img_info.memory_addr      = ctx->dev_addr[0];
  load_img_info.img_rotate       = IT8951_ROTATE_0;
  load_img_info.endian_type      = IT8951_LDIMG_B_ENDIAN;

  if (img_bpp == IMG_1BPP)
    {
      area_img_info.x = img_pos_x / 8;
      area_img_info.y = img_pos_y;
      area_img_info.w = img_width / 8;
      area_img_info.h = img_height;

      load_img_info.img_pixel_format = IT8951_8BPP; /* it's not a typo! */
      status = it8951_packed_pixel_write_1bpp (ctx, 0,
                                               &load_img_info,
                                               &area_img_info);
    }
  else if (img_bpp == IMG_8BPP)
    {
      area_img_info.x = img_pos_x;
      area_img_info.y = img_pos_y;
      area_img_info.w = img_width;
      area_img_info.h = img_height;

      load_img_info.img_pixel_format = IT8951_8BPP;
      status = it8951_packed_pixel_write_8bpp (ctx, 0,
                                               &load_img_info,
                                               &area_img_info);
    }

  if (status == USEC_DEV_OK)
    {
      usec_dev_log ("[usec] status: uploading image - " \
                    "%04d [pos_x] %04d [pos_y] %04d [width] %04d [height]\n\r",
                    img_pos_x, img_pos_y, img_width, img_height);
    }
  else
    {
      usec_dev_log ("[usec] error: cannot upload image data\n\r");
    }

  return status;
}

/*
 * usec_img_update()
 */
uint8_t
usec_img_update (usec_ctx  *ctx,
                 uint32_t   area_pos_x,
                 uint32_t   area_pos_y,
                 uint32_t   area_width,
                 uint32_t   area_height,
                 uint8_t    update_mode,
                 uint8_t    update_wait)
{
  uint8_t status;

  if (ctx == NULL)
    {
      usec_dev_log ("[usec] error: invalid device context\n\r");
      return USEC_DEV_ERR;
    }

  if (update_mode > UPDATE_MODE_DU4)
    {
      usec_dev_log ("[usec] error: invalid update mode value\n\r");
      return USEC_DEV_ERR;
    }

  if ((area_width == 0) || (area_height == 0))
    {
      usec_dev_log ("[usec] error: invalid update area width/height\n\r");
      return USEC_DEV_ERR;
    }

  if ((area_pos_x > ctx->dev_width[0]) || \
      (area_pos_y > ctx->dev_height[0]))
    {
      usec_dev_log ("[usec] error: update area out of screen bounds\n\r");
      return USEC_DEV_ERR;
    }

  if ((area_pos_x + area_width > ctx->dev_width[0]) || \
      (area_pos_y + area_height > ctx->dev_height[0]))
    {
      usec_dev_log ("[usec] error: update area out of screen bounds\n\r");
      return USEC_DEV_ERR;
    }

  /* wait for display */
  status = it8951_display_area (ctx, 0, area_pos_x, area_pos_y,
                                area_width, area_height, update_mode);
  if (status == USEC_DEV_OK)
    {
      usec_dev_log ("[usec] status: updating area - " \
                    "%04d [pos_x] %04d [pos_y] %04d [width] %04d [height]\n\r",
                    area_pos_x, area_pos_y, area_width, area_height);
    }
  else
    {
       usec_dev_log ("[usec] error: cannot update selected display area\n\r");
    }

  /* wait for display */
  if (update_wait)
    it8951_wait_display_ready (ctx, 0);

  /* turn off pmic */
  status |= it8951_pmic_power_off (ctx, 0);

  return status;
}

/******************************************************************************/
