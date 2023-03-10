#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "../console/console.h"
#include "driver/gpio.h"
#include "soc/gpio_struct.h"
#include "soc/adc_channel.h"

#include "adc.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

/*
0 - 107
3200 - 1500
4090 - 1970
*/
/*
4.1 - �� �������
4.07 - ��������
4 - 100%
3.6 - 50%
3.2 - ��������
*/
#define A	1.89
#define B	360

#define ADC_BATTERY_CHANNEL		ADC1_GPIO35_CHANNEL
#define NO_OF_SAMPLES			4


static esp_adc_cal_characteristics_t *adc_chars;

static void check_efuse()
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
    	console_printf(MsgInfo, " eFuse Two Point: Supported\r\n");
    else
    	console_printf(MsgInfo, " eFuse Two Point: NOT supported\r\n");

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
    	console_printf(MsgInfo, " eFuse Vref: Supported\r\n");
    else
    	console_printf(MsgInfo, " eFuse Vref: NOT supported\r\n");

    console_pause(100);
}


static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
    	console_printf(MsgInfo, " Characterized using Two Point Value\r\n");
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
    	console_printf(MsgInfo, " Characterized using eFuse Vref\r\n");
    else
    	console_printf(MsgInfo, " Characterized using Default Vref\r\n");

    console_pause(100);
}


uint32_t getBatteryVoltage()
{
	uint32_t adc_reading = 0;

    for (int i = 0; i < NO_OF_SAMPLES; i++)
        adc_reading += adc1_get_raw(ADC_BATTERY_CHANNEL);

    adc_reading /= NO_OF_SAMPLES;
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    return A * voltage + B;

	return 0;
}


esp_err_t init_adc()
{
	esp_err_t err = ESP_OK;
	uint16_t Vref = 1100;

	console_printf(MsgInfo, "������������� ��� ��� ��������� VBAT (ADC1, channel=%d, 0 db, Vref=%d V)\r\n", ADC_BATTERY_CHANNEL, Vref);
	console_pause(100);

    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    //Configure ADC1
    err = adc1_config_width(ADC_WIDTH_BIT_12);
    if (err != ESP_OK)
    {
    	console_printf(MsgError, "������� adc1_config_width() ������� ��� ������: %d\r\n", err);
    	return err;
    }

    err = adc1_config_channel_atten(ADC_BATTERY_CHANNEL, ADC_ATTEN_DB_6);
    if (err != ESP_OK)
    {
    	console_printf(MsgError, "������� adc1_config_channel_atten() ������� ��� ������: %d\r\n", err);
    	return err;
    }

    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_6, ADC_WIDTH_BIT_12, Vref, adc_chars);
    print_char_val_type(val_type);

    console_printf(MsgInfo, "��� ��� ��������� VBAT ���������������\r\n");
	console_pause(300);

	return err;
}

