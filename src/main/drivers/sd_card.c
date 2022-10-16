#include "drivers/sd_card.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "log.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_defs.h"

#define PIN_NUM_MISO      (13)
#define PIN_NUM_MOSI      (15)
#define PIN_NUM_CLK       (14)
#define PIN_NUM_CS        (4)
#define SPI_DMA_CHAN    host.slot

static char* sd_card_mount_point;
static sdmmc_card_t* sd_card_dev;
static bool sd_card_correctly_mounted;

STATUS sd_card_init(char* mount_point)
{
  esp_err_t ret;
  STATUS s;

  s = STATUS_OK;
  esp_vfs_fat_sdmmc_mount_config_t mount_config =
  {
    .format_if_mount_failed = false,
    .max_files = 10,
    .allocation_unit_size = 16 * 1024
  };

  sd_card_mount_point = mount_point;

  log_info_print("Initializing SD card");

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  spi_bus_config_t bus_cfg =
  {
    .mosi_io_num = PIN_NUM_MOSI,
    .miso_io_num = PIN_NUM_MISO,
    .sclk_io_num = PIN_NUM_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 4000,
  };

  ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
  if (ret != ESP_OK)
  {
    s = STATUS_ERROR;
    log_info_print("Failed to initialize bus.");
  }

  if (s == STATUS_OK)
  {
    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    log_info_print("Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &sd_card_dev);
    if (ret != ESP_OK)
    {
      if (ret == ESP_FAIL)
      {
        log_info_print("Failed to mount filesystem. ");
      }
      else
      {
        log_info_print("Failed to initialize the card (%s). ", esp_err_to_name(ret));
      }
      s = STATUS_ERROR;
    }
    else
      log_info_print("Filesystem mounted");
  }

  if (s == STATUS_OK)
  {
    sdmmc_card_print_info(stdout, sd_card_dev);
    sd_card_correctly_mounted = true;
  }
  return s;
}

void sd_card_umount(void)
{
  if (sd_card_correctly_mounted)
    esp_vfs_fat_sdcard_unmount(sd_card_mount_point, sd_card_dev);
}


STATUS sd_card_get_infos(sd_cart_infos_t* pSdCardInfos)
{
  STATUS s;
  s = STATUS_ERROR;
  if (sd_card_correctly_mounted)
  {
    pSdCardInfos->mounted = true;
    pSdCardInfos->capacity = ((uint64_t) sd_card_dev->csd.capacity) * sd_card_dev->csd.sector_size / (1024 * 1024);
    pSdCardInfos->type = (sd_card_dev->ocr & SD_OCR_SDHC_CAP) ? "SDHC/SDXC" : "SDSC";
    pSdCardInfos->name = sd_card_dev->cid.name;
    s = STATUS_OK;
  }

  return s;
}



