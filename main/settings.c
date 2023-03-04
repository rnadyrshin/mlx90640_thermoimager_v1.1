#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "display/dispcolor.h"
#include "task_ui.h"
#include "main.h"
#include "settings.h"


eButtonFunc FuncUp = Markers_OnOff;//Scale_Next;//Brightness_Plus;// Save_BMP16;
eButtonFunc FuncDown = Scale_Prev;//Brightness_Minus;//Save_CSV;

uint8_t AutoScaleMode = 1;					// Режим автоподстройки шкалы
int32_t LcdBrightness = 100;
int32_t LcdBrightnessOld = 100;
eColorScale ColorScale = Iron;
eScaleMode ScaleMode = LineInterpol;
uint8_t TempMarkers = 1;
float Emissivity = 0.95;	// Отражательная способность поверхности

const float FPS_rates[] = {0.5, 1, 2, 4, 8, 16, 32, 64};
uint8_t FPS_Idx = 5;
uint8_t FPS_Idx_Old = 5;

static nvs_handle SettingsHandle;


int settings_storage_init(void)
{
	esp_err_t err;

    // Initialize NVS
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        err = nvs_flash_erase();
        if (err != ESP_OK)
        	return err;
        // Retry nvs_flash_init
        err = nvs_flash_init();
        if (err != ESP_OK)
        	return err;
    }

    if (nvs_open("settings", NVS_READWRITE, &SettingsHandle) != ESP_OK)
		return -1;

    return 0;
}

int32_t settings_commit(void)
{
    return nvs_commit(SettingsHandle);
}

int setting_read(char *pKey, eType type, void *pValue)
{
    size_t len;

    switch (type)
	{
	case int8: 		return nvs_get_i8(SettingsHandle, pKey, (int8_t *)pValue);
	case uint8:	    return nvs_get_u8(SettingsHandle, pKey, (uint8_t *)pValue);
	case int16:	    return nvs_get_i16(SettingsHandle, pKey, (int16_t *)pValue);
	case uint16:    return nvs_get_u16(SettingsHandle, pKey, (uint16_t *)pValue);
	case int32: 	return nvs_get_i32(SettingsHandle, pKey, (int32_t *)pValue);
	case float32:
	case uint32: 	return nvs_get_u32(SettingsHandle, pKey, (uint32_t *)pValue);
	case int64: 	return nvs_get_i64(SettingsHandle, pKey, (int64_t *)pValue);
	case uint64: 	return nvs_get_u64(SettingsHandle, pKey, (uint64_t *)pValue);
	case str:	    return nvs_get_str(SettingsHandle, pKey, (char *)pValue, &len);
	}

    return 0;
}

int setting_write(char *pKey, eType type, void *pValue)
{
    esp_err_t err = ESP_OK;

    switch (type)
	{
	case int8: 		err = nvs_set_i8(SettingsHandle, pKey, *((int8_t *)pValue)); break;
	case uint8:	    err = nvs_set_u8(SettingsHandle, pKey, *((uint8_t *)pValue)); break;
	case int16:	    err = nvs_set_i16(SettingsHandle, pKey, *((int16_t *)pValue)); break;
	case uint16:    err = nvs_set_u16(SettingsHandle, pKey, *((uint16_t *)pValue));	break;
	case int32: 	err = nvs_set_i32(SettingsHandle, pKey, *((int32_t *)pValue)); break;
	case float32:
	case uint32: 	err = nvs_set_u32(SettingsHandle, pKey, *((uint32_t *)pValue));	break;
	case int64: 	err = nvs_set_i64(SettingsHandle, pKey, *((int64_t *)pValue)); break;
	case uint64: 	err = nvs_set_u64(SettingsHandle, pKey, *((uint64_t *)pValue));	break;
	case str:	    err = nvs_set_str(SettingsHandle, pKey, ((char *)pValue));	break;
	}

    if (err != ESP_OK)
    	return err;

    return 0;
}


int settings_read_all(void)
{
    esp_err_t err = ESP_OK;

    if (!SettingsHandle)	// Хендл ещё не инициализирован
    {
    	if (nvs_open("settings", NVS_READWRITE, &SettingsHandle) != ESP_OK)
    		return -1;
    }


    err = setting_read("Emissivity", float32, &Emissivity);
    if (err != ESP_OK)
    	return err;
    err = setting_read("ScaleMode", uint8, &ScaleMode);
    if (err != ESP_OK)
    	return err;
    err = setting_read("FPS_Idx", uint8, &FPS_Idx);
    if (err != ESP_OK)
    	return err;
    err = setting_read("AutoScaleMode", uint8, &AutoScaleMode);
    if (err != ESP_OK)
    	return err;
    err = setting_read("minTempNew", float32, &minTempNew);
    if (err != ESP_OK)
    	return err;
    err = setting_read("maxTempNew", float32, &maxTempNew);
    if (err != ESP_OK)
    	return err;
    err = setting_read("TempMarkers", uint8, &TempMarkers);
    if (err != ESP_OK)
    	return err;
    err = setting_read("ColorScale", uint8, &ColorScale);
    if (err != ESP_OK)
    	return err;
    err = setting_read("LcdBrightness", int32, &LcdBrightness);
    if (err != ESP_OK)
    	return err;
    err = setting_read("FuncUp", uint8, &FuncUp);
    if (err != ESP_OK)
    	return err;
    err = setting_read("FuncDown", uint8, &FuncDown);
    if (err != ESP_OK)
    	return err;

    return 0;
}

int settings_write_all(void)
{
    esp_err_t err = ESP_OK;

    if (!SettingsHandle)	// Хендл ещё не инициализирован
    {
    	if (nvs_open("settings", NVS_READWRITE, &SettingsHandle) != ESP_OK)
    		return -1;
    }

    err = setting_write("Emissivity", float32, &Emissivity);
    if (err != ESP_OK)
    	return err;
    err = setting_write("ScaleMode", uint8, &ScaleMode);
    if (err != ESP_OK)
    	return err;
    err = setting_write("FPS_Idx", uint8, &FPS_Idx);
    if (err != ESP_OK)
    	return err;
    err = setting_write("AutoScaleMode", uint8, &AutoScaleMode);
    if (err != ESP_OK)
    	return err;
    err = setting_write("minTempNew", float32, &minTempNew);
    if (err != ESP_OK)
    	return err;
    err = setting_write("maxTempNew", float32, &maxTempNew);
    if (err != ESP_OK)
    	return err;
    err = setting_write("TempMarkers", uint8, &TempMarkers);
    if (err != ESP_OK)
    	return err;
    err = setting_write("ColorScale", uint8, &ColorScale);
    if (err != ESP_OK)
    	return err;
    err = setting_write("LcdBrightness", int32, &LcdBrightness);
    if (err != ESP_OK)
    	return err;
    err = setting_write("FuncUp", uint8, &FuncUp);
    if (err != ESP_OK)
    	return err;
    err = setting_write("FuncDown", uint8, &FuncDown);
    if (err != ESP_OK)
    	return err;

    return settings_commit();
}
