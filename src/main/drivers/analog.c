#include "drivers/analog.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "log.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

typedef struct
{
  uint32_t gpio_id;
  adc_unit_t adc_no;
  adc_channel_t channel;
} adc_def_t;

static adc_def_t adc_mapping[] =
{
  { .gpio_id = 32, .adc_no = ADC_UNIT_1, .channel = ADC_CHANNEL_4 },
  { .gpio_id = 33, .adc_no = ADC_UNIT_1, .channel = ADC_CHANNEL_5 },
  { .gpio_id = 34, .adc_no = ADC_UNIT_1, .channel = ADC_CHANNEL_6 },
  { .gpio_id = 35, .adc_no = ADC_UNIT_1, .channel = ADC_CHANNEL_7 },
  { .gpio_id = 36, .adc_no = ADC_UNIT_1, .channel = ADC_CHANNEL_0 },
  { .gpio_id = 37, .adc_no = ADC_UNIT_1, .channel = ADC_CHANNEL_1 },
  { .gpio_id = 38, .adc_no = ADC_UNIT_1, .channel = ADC_CHANNEL_2 },
  { .gpio_id = 39, .adc_no = ADC_UNIT_1, .channel = ADC_CHANNEL_3 },
};

static esp_adc_cal_characteristics_t adc1_chars;
static esp_adc_cal_characteristics_t adc2_chars;

static adc_def_t* analog_get_adc_charac(uint32_t gpio_id);
static void analog_print_char_val_type(esp_adc_cal_value_t val_type);
static void analog_setup_adc_chars(adc_unit_t unit, adc_atten_t attenuation, adc_bits_width_t width);

STATUS analog_configure(uint32_t gpio_id, adc_bits_width_t width, adc_atten_t attenuation, void** handle)
{
  STATUS s;
  adc_def_t* adc_def;
  esp_err_t err;

  s = STATUS_OK;
  adc_def = analog_get_adc_charac(gpio_id);
  if (adc_def == NULL)
  {
    s = STATUS_ERROR;
    *handle = NULL;
  }
  else
  {
    *handle = adc_def;
    switch (adc_def->adc_no)
    {
      case ADC_UNIT_1:
        err = adc1_config_width(width);
        if (err != ESP_OK)
        {
          log_error_print("error trying configure adc1 width %d\n", err);
          s = STATUS_ERROR;
        }

        if (s == STATUS_OK)
        {
          err = adc1_config_channel_atten(adc_def->channel, attenuation);
          if (err != ESP_OK)
          {
            log_error_print("error trying configure adc1 channel atten %d\n", err);
            s = STATUS_ERROR;
          }
        }

        if (s == STATUS_OK)
          analog_setup_adc_chars(adc_def->adc_no, attenuation, width);
        break;

      case ADC_UNIT_2:
        //err = adc2_config_channel_atten(adc_def->channel, attenuation);
        s = STATUS_ERROR;
        break;

      default:
        s = STATUS_ERROR;
        break;
    }
  }
  return s;
}

static adc_def_t* analog_get_adc_charac(uint32_t gpio_id)
{
  adc_def_t* r;
  r = NULL;
  for (uint32_t i = 0; (i < sizeof(adc_mapping) / sizeof(adc_def_t)); i++)
  {
    if (adc_mapping[i].gpio_id == gpio_id)
    {
      r = &adc_mapping[i];
      break;
    }
  }

  return r;
}

void analog_display_efuse_config(void)
{
#if CONFIG_IDF_TARGET_ESP32
  //Check if TP is burned into eFuse
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
  {
    log_dbg_print("eFuse Two Point: Supported\n");
  }
  else
  {
    log_dbg_print("eFuse Two Point: NOT supported\n");
  }
  //Check Vref is burned into eFuse
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
  {
    log_dbg_print("eFuse Vref: Supported\n");
  }
  else
  {
    log_dbg_print("eFuse Vref: NOT supported\n");
  }
#elif CONFIG_IDF_TARGET_ESP32S2
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
  {
    log_dbg_print("eFuse Two Point: Supported\n");
  }
  else
  {
    log_dbg_print("Cannot retrieve eFuse Two Point calibration values. Default calibration values will be used.\n");
  }
#else
#error "This example is configured for ESP32/ESP32S2."
#endif
}

static void analog_print_char_val_type(esp_adc_cal_value_t val_type)
{
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
  {
    log_dbg_print("Characterized using Two Point Value\n");
  }
  else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
  {
    log_dbg_print("Characterized using eFuse Vref\n");
  }
  else
  {
    log_dbg_print("Characterized using Default Vref\n");
  }
}

static void analog_setup_adc_chars(adc_unit_t unit, adc_atten_t attenuation, adc_bits_width_t width)
{
  esp_adc_cal_value_t val_type;
  if (unit == ADC_UNIT_1)
  {
    val_type = esp_adc_cal_characterize(unit, attenuation, width, DEFAULT_VREF, &adc1_chars);
    analog_print_char_val_type(val_type);
  }
  else if (unit == ADC_UNIT_2)
  {
    val_type = esp_adc_cal_characterize(unit, attenuation, width, DEFAULT_VREF, &adc2_chars);
    analog_print_char_val_type(val_type);
  }

}

uint32_t analog_do_conversion(void* handle)
{
  adc_def_t* adc_def = handle;
  esp_adc_cal_characteristics_t* pAdcChars;

  uint32_t voltage;
  //Continuously sample ADC1
  uint32_t adc_reading = 0;
  //Multisampling
  for (int i = 0; i < NO_OF_SAMPLES; i++)
  {
    if (adc_def->adc_no == ADC_UNIT_1)
    {
      adc_reading += adc1_get_raw((adc1_channel_t)adc_def->channel);
    }
    else if (adc_def->adc_no == ADC_UNIT_2)
    {
      ;//TODO
    }
  }

  adc_reading /= NO_OF_SAMPLES;
  //Convert adc_reading to voltage in mV

  if (adc_def->adc_no == ADC_UNIT_1)
    pAdcChars = &adc1_chars;
  else
    pAdcChars = &adc2_chars;

  voltage = esp_adc_cal_raw_to_voltage(adc_reading, pAdcChars);
  log_info_print("Raw: %d\tVoltage: %dmV (0x%.4x)\n", adc_reading, voltage, voltage);
  return voltage;
}

