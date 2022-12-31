#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "log.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "ble_gatt_svr.h"
#include "ble_env_srv.h"
#include "ble_date_srv.h"
#include "bt_uuid.h"
#include "config.h"
#include "data_ope.h"
#include "libs.h"
#include "drivers/pcf_8523.h"

#define BLE_GATT_SET_RC(rc)     ((rc == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES)

static const char* manufacture_name = "miko53 ESP32 meteo station";
static const char* model_number_string = "p1";
static const char* serial_number_string = "0001";
static const char* firmware_rev_string = "0";
static const char* hardware_rev_string = "04/09/2022";
static const char* software_rev_string = METEO_STATION_VERSION;

uint16_t hrs_hrm_handle;
uint16_t handle_true_wind_speed;
uint16_t handle_true_wind_dir;
uint16_t handle_rainfall;
uint16_t handle_temperature;
uint16_t handle_humidity;
uint16_t handle_pressure;
uint16_t handle_current_time;

static int gatt_svr_chr_access_device_info(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg);

static int gatt_srv_get_rainfall(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt,
                                 void* arg);
static int gatt_srv_get_wind_dir(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt,
                                 void* arg);
static int gatt_srv_get_wind_speed(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt,
                                   void* arg);
static int gatt_srv_get_temperature(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt,
                                    void* arg);
static int gatt_srv_get_humidity(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt,
                                 void* arg);
static int gatt_srv_get_pressure(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt,
                                 void* arg);
static int gatt_srv_current_time(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt,
                                 void* arg);
static int ble_gatt_build_es_descriptor_wind_speed(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg);
static int ble_gatt_build_es_descriptor_wind_dir(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg);
static int ble_gatt_build_es_descriptor_rain_fall(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg);
static int ble_gatt_build_es_descriptor_temperature(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg);
static int ble_gatt_build_es_descriptor_humidity(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg);
static int ble_gatt_build_es_descriptor_pressure(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg);


static const struct ble_gatt_svc_def ble_gatt_services[] =
{
  {
    /* Service: Device Information */
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_DEVICE_INFO_SERVICE),
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
  // Environment profile
  {
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = BLE_UUID16_DECLARE(BT_UUID_ENV_SERVICE),
    .characteristics = (struct ble_gatt_chr_def[])
    {
      {
        .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_TRUE_WIND_SPEED),
        .access_cb = gatt_srv_get_wind_speed,
        .flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_READ,
        .val_handle = &handle_true_wind_speed,
        .descriptors = (struct ble_gatt_dsc_def[])
        {
          {
            .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_ES_MEASUREMENT_DESCRIPTOR),
            .att_flags = BLE_ATT_F_READ,
            .access_cb = ble_gatt_build_es_descriptor_wind_speed,
            .arg = NULL,
          },
          /*{
              .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_CHARACTERISTIC_USER_DESCRIPTION),
              .att_flags = BLE_ATT_F_READ | BLE_ATT_F_WRITE,
              .access_cb = ble_gatt_build_es_descriptor,
              .arg = NULL,
          },*/
          { 0 }
        },
      },
      {
        .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_TRUE_WIND_DIR),
        .access_cb = gatt_srv_get_wind_dir,
        .flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_READ,
        .val_handle = &handle_true_wind_dir,
        .descriptors = (struct ble_gatt_dsc_def[])
        {
          {
            .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_ES_MEASUREMENT_DESCRIPTOR),
            .att_flags = BLE_ATT_F_READ,
            .access_cb = ble_gatt_build_es_descriptor_wind_dir,
            .arg = NULL,
          },
          { 0 }
        },
      },
      {
        .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_RAINFALL),
        .access_cb = gatt_srv_get_rainfall,
        .flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_READ,
        .val_handle = &handle_rainfall,
        .descriptors = (struct ble_gatt_dsc_def[])
        {
          {
            .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_ES_MEASUREMENT_DESCRIPTOR),
            .att_flags = BLE_ATT_F_READ,
            .access_cb = ble_gatt_build_es_descriptor_rain_fall,
            .arg = NULL,
          },

          { 0 }
        },
      },
      {
        .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_TEMPERATURE),
        .access_cb = gatt_srv_get_temperature,
        .flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_READ,
        .val_handle = &handle_temperature,
        .descriptors = (struct ble_gatt_dsc_def[])
        {
          {
            .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_ES_MEASUREMENT_DESCRIPTOR),
            .att_flags = BLE_ATT_F_READ,
            .access_cb = ble_gatt_build_es_descriptor_temperature,
            .arg = NULL,
          },

          { 0 }
        },
      },
      {
        .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_HUMIDITY),
        .access_cb = gatt_srv_get_humidity,
        .flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_READ,
        .val_handle = &handle_humidity,
        .descriptors = (struct ble_gatt_dsc_def[])
        {
          {
            .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_ES_MEASUREMENT_DESCRIPTOR),
            .att_flags = BLE_ATT_F_READ,
            .access_cb = ble_gatt_build_es_descriptor_humidity,
            .arg = NULL,
          },

          { 0 }
        },
      },
      {
        .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_PRESSURE),
        .access_cb = gatt_srv_get_pressure,
        .flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_READ,
        .val_handle = &handle_pressure,
        .descriptors = (struct ble_gatt_dsc_def[])
        {
          {
            .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_ES_MEASUREMENT_DESCRIPTOR),
            .att_flags = BLE_ATT_F_READ,
            .access_cb = ble_gatt_build_es_descriptor_pressure,
            .arg = NULL,
          },

          { 0 }
        },
      },
      {
        0, /* No more characteristics in this service */
      },
    }
  },
  //Time profile
  {
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_CURRENT_TIME_SERVICE),
    .characteristics = (struct ble_gatt_chr_def[])
    {
      {
        .uuid = BLE_UUID16_DECLARE(BT_UUID_GATT_CURRENT_TIME),
        .access_cb = gatt_srv_current_time,
        .flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        .val_handle = &handle_current_time,
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

static void gatt_build_desc(uint32_t indexSensor, uint8_t frame[ES_MEASUREMENT_DESC_LEN])
{
  gatt_es_measurement_desc esDesc;
  data_operation_t* pDataOperation;
  pDataOperation = date_ope_get_operation(indexSensor);

  esDesc.sampling_fun = gatt_desc_get_sampling_func(pDataOperation->operation);
  esDesc.measurement_period = pDataOperation->calcul_period.period_sec;
  esDesc.update_interval = pDataOperation->calcul_period.period_sec;
  esDesc.application = ES_APPLICATION_EXTERNAL;
  esDesc.measurement_incertainty = 0;
  build_frame(frame, &esDesc);
}

static int ble_gatt_build_es_descriptor_rain_fall(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg)
{
  int rc;
  uint8_t frame[ES_MEASUREMENT_DESC_LEN];
  gatt_build_desc(0, frame); //TODO
  rc = os_mbuf_append(ctxt->om, frame, ES_MEASUREMENT_DESC_LEN);
  return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int ble_gatt_build_es_descriptor_wind_speed(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg)
{
  int rc;
  uint8_t frame[ES_MEASUREMENT_DESC_LEN];
  gatt_build_desc(2, frame); //TODO
  rc = os_mbuf_append(ctxt->om, frame, ES_MEASUREMENT_DESC_LEN);
  return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int ble_gatt_build_es_descriptor_wind_dir(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg)
{
  int rc;
  uint8_t frame[ES_MEASUREMENT_DESC_LEN];
  gatt_build_desc(4, frame); //TODO
  rc = os_mbuf_append(ctxt->om, frame, ES_MEASUREMENT_DESC_LEN);
  return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int ble_gatt_build_es_descriptor_temperature(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg)
{
  return BLE_ATT_ERR_UNLIKELY;
}

static int ble_gatt_build_es_descriptor_humidity(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg)
{
  return BLE_ATT_ERR_UNLIKELY;
}

static int ble_gatt_build_es_descriptor_pressure(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt, void* arg)
{
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

static int gatt_srv_get_rainfall(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt,
                                 void* arg)
{
  histogram_t* currentHisto;
  currentHisto = data_ope_get_histo(SENSOR_INDEX_SLIDE_RAIN_FALL);

  variant_t v;
  STATUS s;
  int rc;
  rc = BLE_ATT_ERR_UNLIKELY;
  s = histogram_get(currentHisto, LAST_VALUE, &v);
  if (s == STATUS_OK)
  {
    uint32_t d;
    d = v.f32;
    rc = os_mbuf_append(ctxt->om, &d, sizeof(uint32_t));
  }

  return BLE_GATT_SET_RC(rc);
}


static int gatt_srv_get_wind_speed(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt,
                                   void* arg)
{
  histogram_t* currentHisto;
  currentHisto = data_ope_get_histo(SENSOR_INDEX_SLIDE_WIND_SPEED);

  variant_t v;
  STATUS s;
  int rc;
  rc = BLE_ATT_ERR_UNLIKELY;
  s = histogram_get(currentHisto, LAST_VALUE, &v);
  if (s == STATUS_OK)
  {
    uint32_t d;
    d = v.f32 * 100;
    rc = os_mbuf_append(ctxt->om, &d, sizeof(uint32_t));
  }

  return BLE_GATT_SET_RC(rc);
}


static int gatt_srv_get_wind_dir(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt,
                                 void* arg)
{
  histogram_t* currentHisto;
  currentHisto = data_ope_get_histo(SENSOR_INDEX_SLIDE_WIND_DIR);

  variant_t v;
  STATUS s;
  int rc;
  rc = BLE_ATT_ERR_UNLIKELY;
  s = histogram_get(currentHisto, LAST_VALUE, &v);
  if (s == STATUS_OK)
  {
    uint32_t d;
    d = v.i32 * 100;
    rc = os_mbuf_append(ctxt->om, &d, sizeof(uint32_t));
  }

  return BLE_GATT_SET_RC(rc);
}

static int gatt_srv_get_temperature(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt,
                                    void* arg)
{
  int rc;
  rc = BLE_ATT_ERR_UNLIKELY;
  return rc;
}

static int gatt_srv_get_humidity(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt,
                                 void* arg)
{
  int rc;
  rc = BLE_ATT_ERR_UNLIKELY;
  return rc;
}

static int gatt_srv_get_pressure(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt,
                                 void* arg)
{
  int rc;
  rc = BLE_ATT_ERR_UNLIKELY;
  return rc;
}

static int gatt_svr_chr_write(struct os_mbuf* om, uint16_t min_len, uint16_t max_len, void* dst, uint16_t* len)
{
  uint16_t om_len;
  int rc;

  om_len = OS_MBUF_PKTLEN(om);
  if (om_len < min_len || om_len > max_len)
  {
    return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
  }

  rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
  if (rc != 0)
  {
    return BLE_ATT_ERR_UNLIKELY;
  }

  return 0;
}

int gatt_srv_current_time ( uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg )
{
  uint8_t frame[BLE_DATE_FRAME_SIZE];
  int rc;
  rc = -1;
  if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR)
  {
    ble_date_build_frame(frame);
    rc = os_mbuf_append(ctxt->om, frame, sizeof(frame));
  }
  else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
  {
    uint16_t len = 0;
    rc = gatt_svr_chr_write(ctxt->om, 10, 10, frame, &len);
    if (rc == 0)
    {
      struct tm localTime;
      ble_date_decode(&localTime, frame);
      pcf8523_set_date(&localTime);
      date_set_localtime(&localTime);
    }
    else
    {
      return rc;
    }
  }

  return BLE_GATT_SET_RC(rc);
}
