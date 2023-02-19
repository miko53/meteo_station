#include "nvstorage.h"
#include "os.h"
#include "nvs_flash.h"
#include "log.h"

static nvs_handle_t nvstorage_handle;

STATUS nvstorage_init(void)
{
  STATUS status;
  status = STATUS_ERROR;

  esp_err_t err;
  err = nvs_flash_init();

  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK( err );

  if (err == ESP_OK)
  {
    err = nvs_open("storage", NVS_READWRITE, &nvstorage_handle);
  }

  if (err == ESP_OK)
    status = STATUS_OK;

  return status;
}

bool nvstorage_get_sdcard_log_state(void)
{
  esp_err_t err;
  uint32_t v;

  v = false;
  err = nvs_get_u32(nvstorage_handle, "[sdcard_log]", &v);
  if (err != ESP_OK)
  {
    log_dbg_print("read status = %d\n", err);
  }

  return (bool) v;
}

STATUS nvstorage_set_sdcard_log_state(bool bState)
{
  STATUS s;
  esp_err_t err;
  s = STATUS_ERROR;

  err = nvs_set_u32 (nvstorage_handle, "[sdcard_log]", bState);
  if (err == ESP_OK)
    s = STATUS_OK;

  return s;
}

bool nvstorage_get_ble_state(void)
{
  esp_err_t err;
  uint32_t v;

  v = false;
  err = nvs_get_u32(nvstorage_handle, "[ble]", &v);
  if (err != ESP_OK)
  {
    log_dbg_print("read status = %d\n", err);
  }

  return (bool) v;
}

STATUS nvstorage_set_ble_state(bool bState)
{
  STATUS s;
  esp_err_t err;
  s = STATUS_ERROR;

  err = nvs_set_u32 (nvstorage_handle, "[ble]", bState);
  if (err == ESP_OK)
    s = STATUS_OK;

  return s;
}

bool nvstorage_get_zb_state(void)
{
  esp_err_t err;
  uint32_t v;

  v = false;
  err = nvs_get_u32(nvstorage_handle, "[zb]", &v);
  if (err != ESP_OK)
  {
    log_dbg_print("read status = %d\n", err);
  }

  return (bool) v;
}

STATUS nvstorage_set_zb_state(bool bState)
{
  STATUS s;
  esp_err_t err;
  s = STATUS_ERROR;

  err = nvs_set_u32 (nvstorage_handle, "[zb]", bState);
  if (err == ESP_OK)
    s = STATUS_OK;

  return s;
}
