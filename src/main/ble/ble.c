#include "ble.h"
#include "log.h"
#include "esp_nimble_hci.h"
#include "config.h"

#include "freertos/FreeRTOSConfig.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "ble_gatt_svr.h"


static const char* ble_device_name = "meteo_station_" METEO_STATION_VERSION;
static uint8_t ble_address_type;

/* Variable to simulate heart beats */
static uint8_t heartrate = 90;

static TimerHandle_t blehr_tx_timer;
static bool notify_state;
static uint16_t conn_handle;


static void ble_on_synchronization(void);
static void ble_on_reset(int reason);
static void ble_nimble_srv_task(void* param);
static void ble_prepare_and_start_advertise(void);

static int ble_gap_event(struct ble_gap_event* event, void* arg);

static void blehr_tx_hrate_stop(void)
{
  xTimerStop( blehr_tx_timer, 1000 / portTICK_PERIOD_MS );
}

/* Reset heart rate measurement */
static void blehr_tx_hrate_reset(void)
{
  int rc;

  if (xTimerReset(blehr_tx_timer, 1000 / portTICK_PERIOD_MS ) == pdPASS)
  {
    rc = 0;
  }
  else
  {
    rc = 1;
  }

  assert(rc == 0);
}

/* This function simulates heart beat and notifies it to the client */
static void blehr_tx_hrate(TimerHandle_t ev)
{
  static uint8_t hrm[2];
  int rc;
  struct os_mbuf* om;

  if (!notify_state)
  {
    blehr_tx_hrate_stop();
    heartrate = 90;
    return;
  }

  hrm[0] = 0x06; /* contact of a sensor */
  hrm[1] = heartrate; /* storing dummy data */

  /* Simulation of heart beats */
  heartrate++;
  if (heartrate == 160)
  {
    heartrate = 90;
  }

  om = ble_hs_mbuf_from_flat(hrm, sizeof(hrm));
  rc = ble_gattc_notify_custom(conn_handle, hrs_hrm_handle, om);

  assert(rc == 0);

  blehr_tx_hrate_reset();
}



STATUS ble_init(void)
{
  STATUS s;
  esp_err_t espStatus;

  s = STATUS_OK;
  espStatus = esp_nimble_hci_and_controller_init();
  if (espStatus != ESP_OK)
  {
    log_error_print("unable to initialize nimble hci and controller (%d)", espStatus);
    s = STATUS_ERROR;
  }
  else
  {
    nimble_port_init();

    // initialize the NimBLE host configuration
    ble_hs_cfg.sync_cb = ble_on_synchronization;
    ble_hs_cfg.reset_cb = ble_on_reset;

    /* name, period/time,  auto reload, timer ID, callback */
    blehr_tx_timer = xTimerCreate("blehr_tx_timer", pdMS_TO_TICKS(1000), pdTRUE, (void*)0, blehr_tx_hrate); //TODO remove it
    s = ble_gatt_srv_initialize();
  }

  if (s == STATUS_OK)
  {
    /* Set the default device name */
    int rc;
    rc = ble_svc_gap_device_name_set(ble_device_name);
    if (rc != 0)
      s = STATUS_ERROR;
  }

  if (s == STATUS_OK)
  {
    // start nimble service task
    nimble_port_freertos_init(ble_nimble_srv_task);
  }

  return s;
}

static void ble_on_synchronization(void)
{
  uint8_t addr_val[6] = {0};
  int rc;

  rc = ble_hs_id_infer_auto(0, &ble_address_type);
  assert(rc == 0);
  rc = ble_hs_id_copy_addr(ble_address_type, addr_val, NULL);

  log_info_print("ble: on_synchronization, device address: %02x:%02x:%02x:%02x:%02x:%02x",
                 addr_val[5], addr_val[4], addr_val[3], addr_val[2], addr_val[1], addr_val[0]);

  // Begin advertising
  ble_prepare_and_start_advertise();
}

static void ble_on_reset(int reason)
{
  log_error_print("Resetting state; reason=%d", reason);
}

static void ble_nimble_srv_task(void* param)
{
  log_info_print("nimble service started");
  /* This function will return only when nimble_port_stop() is executed */
  nimble_port_run();
  nimble_port_freertos_deinit();
}

/*
 * Enables advertising with parameters:
 *     o General discoverable mode
 *     o Undirected connectable mode
 */
static void ble_prepare_and_start_advertise(void)
{
  struct ble_gap_adv_params adv_params;
  struct ble_hs_adv_fields fields;
  int rc;

  /*
   *  Set the advertisement data included in our advertisements:
   *     o Flags (indicates advertisement type and other general info)
   *     o Advertising tx power
   *     o Device name
   */
  memset(&fields, 0, sizeof(fields));

  /*
   * Advertise two flags:
   *      o Discoverability in forthcoming advertisement (general)
   *      o BLE-only (BR/EDR unsupported)
   */
  fields.flags = BLE_HS_ADV_F_DISC_GEN |
                 BLE_HS_ADV_F_BREDR_UNSUP;

  /*
   * Indicate that the TX power level field should be included; have the
   * stack fill this value automatically.  This is done by assigning the
   * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
   */
  fields.tx_pwr_lvl_is_present = 1;
  fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

  fields.name = (uint8_t*)ble_device_name;
  fields.name_len = strlen(ble_device_name);
  fields.name_is_complete = 1;

  rc = ble_gap_adv_set_fields(&fields);
  if (rc != 0)
  {
    log_error_print("error setting advertisement data; rc=%d\n", rc);
    return;
  }

  /* Begin advertising */
  memset(&adv_params, 0, sizeof(adv_params));
  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
  rc = ble_gap_adv_start(ble_address_type, NULL, BLE_HS_FOREVER,
                         &adv_params, ble_gap_event, NULL);
  if (rc != 0)
  {
    MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
    return;
  }
}

static int ble_gap_event(struct ble_gap_event* event, void* arg)
{
  switch (event->type)
  {
    case BLE_GAP_EVENT_CONNECT:
      /* A new connection was established or a connection attempt failed */
      log_info_print("connection %s; status=%d",
                     event->connect.status == 0 ? "established" : "failed",
                     event->connect.status);

      // Connection failed; resume advertising
      if (event->connect.status != 0)
        ble_prepare_and_start_advertise();

      conn_handle = event->connect.conn_handle;
      break;

    case BLE_GAP_EVENT_DISCONNECT:
      log_info_print("disconnect; reason=%d", event->disconnect.reason);

      // Connection terminated; resume advertising
      ble_prepare_and_start_advertise();
      break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
      log_info_print("adv complete");
      ble_prepare_and_start_advertise();
      break;

    case BLE_GAP_EVENT_SUBSCRIBE:
      log_info_print("subscribe event; cur_notify=%d\n value handle; val_handle=%d",
                     event->subscribe.cur_notify, hrs_hrm_handle);

      if (event->subscribe.attr_handle == hrs_hrm_handle)
      {
        notify_state = event->subscribe.cur_notify;
        blehr_tx_hrate_reset();
      }
      else if (event->subscribe.attr_handle != hrs_hrm_handle)
      {
        notify_state = event->subscribe.cur_notify;
        blehr_tx_hrate_stop();
      }
      log_info_print("BLE_GAP_SUBSCRIBE_EVENT", "conn_handle from subscribe=%d", conn_handle);
      break;

    case BLE_GAP_EVENT_MTU:
      log_info_print("mtu update event; conn_handle=%d mtu=%d",
                     event->mtu.conn_handle,
                     event->mtu.value);
      break;

    default:
      log_info_print("unknown event : %d", event->type);
      break;
  }

  return 0;
}
