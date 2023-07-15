#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "mlx90640/mlx90640_api.h"
#include "adc/adc.h"
#include "settings.h"
#include "main.h"
#include "task_mlx.h"

paramsMLX90640 params;
uint16_t *Frame;
sMlxData MlxData[2];
uint8_t MlxDataIdx = 0;
uint8_t MlxError = 0;

static uint16_t badPixels[] = 
{
	// 10,	// Bad pixel number 1
	// 201,	// Bad pixel number 2
	// 339,	// Bad pixel number 3
	0xFFFF
};

//==============================================================================
// ��������� �������� ������� ���������� � ����� pBuff
//==============================================================================
void GetThermoData(float *pBuff)
{
	sMlxData *pMlxData = &MlxData[(MlxDataIdx + 1) & 1];
	float *ThermoImage = pMlxData->ThermoImage;
	memcpy(pBuff, ThermoImage, (termWidth * termHeight) << 2);
}
//==============================================================================

void mlx_task(void* arg)
{
    // ������ ���������� mlx90640 �� EEPROM
	int result = MLX90640_DumpEE(0x33, Frame); //the whole EEPROM is stored in the eeMLX90640 array

    if (result < 0)
    {
    	MlxError = 1;
		vTaskDelete(NULL);
        return;
    }

    result = MLX90640_ExtractParameters(Frame, &params);
    if (result < 0)
    {
    	MlxError = 1;
		vTaskDelete(NULL);
        return;
    }

    // ��������� ������� ���������� �����������
    result = MLX90640_SetRefreshRate(0x33, FPS_Idx);
    if (result < 0)
    {
    	MlxError = 1;
		vTaskDelete(NULL);
    	return;
    }
	FPS_Idx_Old = FPS_Idx;

	while (1)
    {
		sMlxData *pMlxData = &MlxData[MlxDataIdx];
		float *ThermoImage = pMlxData->ThermoImage;

		pMlxData->VBAT = ((float)getBatteryVoltage()) / 1000;

		if (FPS_Idx != FPS_Idx_Old)
		{
		    // ��������� ������� ���������� �����������
		    result = MLX90640_SetRefreshRate(0x33, FPS_Idx);
		    if (!result)
		    	FPS_Idx_Old = FPS_Idx;
		}

		// ������ 2 ���������� ������ �� �������
        for (uint8_t subPage = 0 ; subPage < 2 ; subPage++)
        {
        	// ������ ������ �� �������
        	result = MLX90640_GetFrameData(0x33, Frame);

        	// ������� � ������� ��������� ����������, ��������� �� mlx90640
        	pMlxData->Vdd = MLX90640_GetVdd(Frame, &params);
        	pMlxData->Ta = MLX90640_GetTa(Frame, &params);

        	// ������ ������� ����������
        	float tr = pMlxData->Ta - 8;
        	MLX90640_CalculateTo(Frame, &params, Emissivity, tr, pMlxData->ThermoImage);
        }

        MLX90640_BadPixelsCorrection(badPixels, pMlxData->ThermoImage, 1, &params);   

    	// ������� ����������� � ������ ������
        pMlxData->CenterTemp =
    			ThermoImage[termWidth * ((termHeight >> 1) - 1) + ((termWidth >> 1) - 1)] +
    			ThermoImage[termWidth * ((termHeight >> 1) - 1) + (termWidth >> 1)] +
    			ThermoImage[termWidth * (termHeight >> 1) + ((termWidth >> 1) - 1)] +
				ThermoImage[termWidth * (termHeight >> 1) + (termWidth >> 1)];
        pMlxData->CenterTemp /= 4;

    	// ����� ����������� � ������������ ����������� � �����
        pMlxData->minT = 300;
        pMlxData->maxT = -40;

		for (uint8_t y = 0; y < termHeight; y++)
		{
			for (uint8_t x = 0; x < termWidth; x++)
			{
				float temp = ThermoImage[y * termWidth + x];
				if (pMlxData->maxT < temp)
				{
					pMlxData->maxT = temp;
					pMlxData->maxT_X = x;
					pMlxData->maxT_Y = y;
				}
				if (pMlxData->minT > temp)
				{
					pMlxData->minT = temp;
					pMlxData->minT_X = x;
					pMlxData->minT_Y = y;
				}
			}
    	}
		if (pMlxData->maxT > MAX_TEMP)
			pMlxData->maxT = MAX_TEMP;
		if (pMlxData->minT < MIN_TEMP)
			pMlxData->minT = MIN_TEMP;

        MlxDataIdx = (MlxDataIdx + 1) & 1;
    }
}
