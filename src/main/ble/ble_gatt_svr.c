#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "log.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "ble_gatt_svr.h"
#include "bt_uuid.h"
#include "config.h"
#include "data_ope.h"
#include "libs.h"
#include "drivers/pcf_8523.h"

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

#define ES_MEASUREMENT_DESC_LEN     (11)

typedef enum
{
  SAMPLING_FUN_UNSPECIFIED = 0x00,
  SAMPLING_FUN_INSTANTANEOUS = 0x01,
  SAMPLING_FUN_ARITHMETIC_MEAN = 0x02,
  SAMPLING_FUN_RMS = 0x03,
  SAMPLING_FUN_MAXIMUM = 0x04,
  SAMPLING_FUN_MINIMUM = 0x05,
  SAMPLING_FUN_ACCUMULATED = 0x06,
  SAMPLING_FUN_COUNT = 0x07,
} es_sampling_fun_t;

typedef enum
{
  ES_APPLICATION_UNSPECIFIED,
  ES_APPLICATION_AIR,
  ES_APPLICATION_WATER,
  ES_APPLICATION_BAROMETRIC,
  ES_APPLICATION_SOIL,
  ES_APPLICATION_INFRARED,
  ES_APPLICATION_MAP_DATABASE,
  ES_APPLICATION_BAROMETRIC_ELEVATION_SOURCE,
  ES_APPLICATION_GPS_ONLY_ELEVATION_SOURCE,
  ES_APPLICATION_GPS_AND_MAP_DATABASE_ELEVATION_SOURCE,
  ES_APPLICATION_VERTICAL_DATUM_ELEVATION_SOURCE,
  ES_APPLICATION_ONSHORE,
  ES_APPLICATION_ONBOARD_VESSEL_OR_VEHICLE,
  ES_APPLICATION_FRONT,
  ES_APPLICATION_BAC_REAR,
  ES_APPLICATION_UPPER,
  ES_APPLICATION_LOWER,
  ES_APPLICATION_PRIMARY,
  ES_APPLICATION_SECONDARY,
  ES_APPLICATION_OUTDOOR,
  ES_APPLICATION_INDOOR,
  ES_APPLICATION_TOP,
  ES_APPLICATION_BOTTOM,
  ES_APPLICATION_MAIN,
  ES_APPLICATION_BACKUP,
  ES_APPLICATION_AUXILIARY,
  ES_APPLICATION_SUPPLEMENTARY,
  ES_APPLICATION_INSIDE,
  ES_APPLICATION_OUTSIDE,
  ES_APPLICATION_LEFT,
  ES_APPLICATION_RIGHT,
  ES_APPLICATION_INTERNAL,
  ES_APPLICATION_EXTERNAL,
  ES_APPLICATION_SOLAR,
} es_application_t;

typedef struct
{
  //uint16_t flags; //< reserved for future use
  es_sampling_fun_t sampling_fun;
  uint32_t measurement_period;
  uint32_t update_interval;
  es_application_t application;
  uint8_t measurement_incertainty;
} gatt_es_measurement_desc;

static void build_frame(uint8_t frame[ES_MEASUREMENT_DESC_LEN], gatt_es_measurement_desc* esDesc)
{
  frame[0] = 0;
  frame[1] = 0;
  frame[2] = esDesc->sampling_fun;
  frame[3] = (esDesc->measurement_period & 0xFF);
  frame[4] = (esDesc->measurement_period & 0xFF00) >> 8;
  frame[5] = (esDesc->measurement_period & 0xFF0000) >> 16;
  frame[6] = (esDesc->update_interval & 0xFF);
  frame[7] = (esDesc->update_interval & 0xFF00) >> 8;
  frame[8] = (esDesc->update_interval & 0xFF0000) >> 16;
  frame[9] = (esDesc->application);
  frame[10] = (esDesc->measurement_incertainty);
}

static es_sampling_fun_t gatt_desc_get_sampling_func(data_calcul dataCalOpe)
{
  es_sampling_fun_t r;

  switch (dataCalOpe)
  {
    case OPE_AVERAGE:
      r = SAMPLING_FUN_ARITHMETIC_MEAN;
      break;
    case OPE_CUMUL:
      r = SAMPLING_FUN_ACCUMULATED;
      break;
    case OPE_MAX:
      r = SAMPLING_FUN_MAXIMUM;
      break;
    case OPE_MIN:
      r = SAMPLING_FUN_MINIMUM;
      break;
    default:
      r = -1;
      break;
  }
  return r;
}

#define BLE_GATT_SET_RC(rc)     ((rc == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES)

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
  currentHisto = data_ope_get_histo(0); //TODO

  variant_t v;
  STATUS s;
  int rc;
  rc = BLE_ATT_ERR_UNLIKELY;
  s = histogram_get(currentHisto, 0, &v);
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
  currentHisto = data_ope_get_histo(2); //TODO

  variant_t v;
  STATUS s;
  int rc;
  rc = BLE_ATT_ERR_UNLIKELY;
  s = histogram_get(currentHisto, 0, &v);
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
  currentHisto = data_ope_get_histo(4); //TODO

  variant_t v;
  STATUS s;
  int rc;
  rc = BLE_ATT_ERR_UNLIKELY;
  s = histogram_get(currentHisto, 0, &v);
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
  uint8_t frame[10];
  int rc;
  rc = -1;
  if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR)
  {
    struct tm localTime;
    date_get_localtime(&localTime);

    localTime.tm_year += 1900;
    localTime.tm_mon += 1;

    frame[0] = localTime.tm_year & 0xFF;
    frame[1] = (localTime.tm_year & 0xFF00) >> 8;
    frame[2] = (localTime.tm_mon);
    frame[3] = (localTime.tm_mday);
    frame[4] = (localTime.tm_hour);
    frame[5] = (localTime.tm_min);
    frame[6] = (localTime.tm_sec);
    frame[7] = 0;
    frame[8] = 0;
    frame[9] = 0;

    rc = os_mbuf_append(ctxt->om, frame, sizeof(frame));
  }
  else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
  {
    uint16_t len = 0;
    rc = gatt_svr_chr_write(ctxt->om, 10, 10, frame, &len);
    if (rc == 0)
    {
      struct tm localTime;
      memset(&localTime, 0, sizeof(struct tm));
      localTime.tm_year = (frame[1] << 8) | frame[0];
      localTime.tm_mon = frame[2];
      localTime.tm_mday = frame[3];
      localTime.tm_hour = frame[4];
      localTime.tm_min = frame[5];
      localTime.tm_sec = frame[6];

      localTime.tm_mon -=  1;
      localTime.tm_year -= 1900;
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
