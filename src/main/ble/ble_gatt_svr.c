#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "ble_gatt_svr.h"
#include "bt_uuid.h"
#include "config.h"

static const char* manufacture_name = "miko53 ESP32 meteo station";
static const char* model_number_string = "p1";
static const char* serial_number_string = "0001";
static const char* firmware_rev_string = "0";
static const char* hardware_rev_string = "04/09/2022";
static const char* software_rev_string = METEO_STATION_VERSION;

uint16_t hrs_hrm_handle;

static int gatt_svr_chr_access_heart_rate(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg);

static int gatt_svr_chr_access_device_info(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg);

static const struct ble_gatt_svc_def ble_gatt_services[] =
{
  {
    /* Service: Heart-rate */
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = BLE_UUID16_DECLARE(GATT_HRS_UUID),
    .characteristics = (struct ble_gatt_chr_def[])
    {
      {
        /* Characteristic: Heart-rate measurement */
        .uuid = BLE_UUID16_DECLARE(GATT_HRS_MEASUREMENT_UUID),
        .access_cb = gatt_svr_chr_access_heart_rate,
        .val_handle = &hrs_hrm_handle,
        .flags = BLE_GATT_CHR_F_NOTIFY,
      },
      {
        /* Characteristic: Body sensor location */
        .uuid = BLE_UUID16_DECLARE(GATT_HRS_BODY_SENSOR_LOC_UUID),
        .access_cb = gatt_svr_chr_access_heart_rate,
        .flags = BLE_GATT_CHR_F_READ,
      },
      {
        0, /* No more characteristics in this service */
      },
    }
  },
  {
    /* Service: Device Information */
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
    .characteristics = (struct ble_gatt_chr_def[])
    {
      {
        .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_MODEL_NUMBER_UUID),
        .access_cb = gatt_svr_chr_access_device_info,
        .flags = BLE_GATT_CHR_F_READ,
      },
      {
        .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_SERIAL_NUMBER_STRING),
        .access_cb = gatt_svr_chr_access_device_info,
        .flags = BLE_GATT_CHR_F_READ,
      },
      {
        .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_FIRMWARE_REV_STRING),
        .access_cb = gatt_svr_chr_access_device_info,
        .flags = BLE_GATT_CHR_F_READ,
      },
      {
        .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_HARDWARE_REV_STRING),
        .access_cb = gatt_svr_chr_access_device_info,
        .flags = BLE_GATT_CHR_F_READ,
      },
      {
        .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_SOFTWARE_REV_STRING),
        .access_cb = gatt_svr_chr_access_device_info,
        .flags = BLE_GATT_CHR_F_READ,
      },
      {
        .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_MANUFACTURER_NAME_UUID),
        .access_cb = gatt_svr_chr_access_device_info,
        .flags = BLE_GATT_CHR_F_READ,
      },
      {
        0, /* No more characteristics in this service */
      },
    }
  },
  {
    0, /* No more services */
  },
};

static int gatt_svr_chr_access_heart_rate(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg)
{
  /* Sensor location, set to "Chest" */
  static uint8_t body_sens_loc = 0x01;
  uint16_t uuid;
  int rc;

  uuid = ble_uuid_u16(ctxt->chr->uuid);

  if (uuid == GATT_HRS_BODY_SENSOR_LOC_UUID)
  {
    rc = os_mbuf_append(ctxt->om, &body_sens_loc, sizeof(body_sens_loc));

    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  }

  assert(0);
  return BLE_ATT_ERR_UNLIKELY;
}

static int gatt_svr_chr_access_device_info(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg)
{
  uint16_t uuid;
  int rc;
  const char* pString = NULL;
  uint32_t size = 0;

  uuid = ble_uuid_u16(ctxt->chr->uuid);
  switch (uuid)
  {
    case BT_UUID_GATT_MODEL_NUMBER_UUID:
      pString = model_number_string;
      size = strlen(model_number_string);
      break;

    case BT_UUID_GATT_SERIAL_NUMBER_STRING:
      pString = serial_number_string;
      size = strlen(serial_number_string);
      break;

    case BT_UUID_GATT_FIRMWARE_REV_STRING:
      pString = firmware_rev_string;
      size = strlen(firmware_rev_string);
      break;

    case BT_UUID_GATT_HARDWARE_REV_STRING:
      pString = hardware_rev_string;
      size = strlen(hardware_rev_string);
      break;

    case BT_UUID_GATT_SOFTWARE_REV_STRING:
      pString = software_rev_string;
      size = strlen(software_rev_string);
      break;

    case BT_UUID_GATT_MANUFACTURER_NAME_UUID:
      pString = manufacture_name;
      size =  strlen(manufacture_name);
      break;

    default:
      rc = BLE_ATT_ERR_UNLIKELY;
      break;
  }

  if (pString != NULL)
    rc = os_mbuf_append(ctxt->om, pString, size);

  return rc;
}

STATUS ble_gatt_srv_initialize(void)
{
  STATUS s;
  int rc;

  s = STATUS_OK;
  ble_svc_gap_init();
  ble_svc_gatt_init();

  rc = ble_gatts_count_cfg(ble_gatt_services);
  if (rc != 0)
  {
    s = STATUS_ERROR;
  }

  if (s == STATUS_OK)
  {
    rc = ble_gatts_add_svcs(ble_gatt_services);
    if (rc != 0)
    {
      s = STATUS_ERROR;
    }
  }

  return s;
}

