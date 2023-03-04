#include "display/dispcolor.h"
#include "display/rgbcolor.h"
#include "palette/palette.h"
#include "hqx/hqx.h"
#include "settings.h"
#include "menu.h"
#include "func.h"
#include "version.h"
#include "task_buttons.h"
#include "task_mlx.h"
#include "task_ui.h"


#define iSteps					9		// Количество промежуточных точек при интерполяции по горизонтали/вертикали


uint8_t Need2RedrawTitle = 0;
uint8_t Need2RedrawPalette = 0;

static int16_t *TermoImage16;
static int16_t *TermoStage1;
static int16_t *TermoHqImage16;
static uint16_t PaletteSteps = 0;
static tRGBcolor *pPaletteImage;
static tRGBcolor pPaletteScale[termHeight * 9];

uint16_t imageHeight;
uint16_t imageWidth;

static float minTemp = 0;
static float maxTemp = 0;
float minTempNew = SCALE_DEFAULT_MIN;
float maxTempNew = SCALE_DEFAULT_MAX;


//==============================================================================
// Процедура отрисовки термограммы в исходном разрешении
//==============================================================================
void DrawImage(int16_t *pImage, tRGBcolor *pPalette, uint16_t PaletteSize, uint16_t X, uint16_t Y, uint8_t pixelWidth, uint8_t pixelHeight, float minTemp)
{
	int cnt = 0;
	for (int row = 0; row < 24; row++)
	{
		for (int col = 0; col < 32; col++, cnt++)
		{
			int16_t colorIdx = pImage[cnt] - (minTemp * 10);

			if (colorIdx < 0)
				colorIdx = 0;
			if (colorIdx >= PaletteSize)
				colorIdx = PaletteSize - 1;

	    	uint16_t color = RGB565(pPalette[colorIdx].r, pPalette[colorIdx].g, pPalette[colorIdx].b);
			dispcolor_FillRect((31 - col) * pixelWidth + X, row * pixelHeight + Y, pixelWidth, pixelHeight, color);
		}
	}
}
//==============================================================================


//==============================================================================
// Процедура отрисовки интерполированной термограммы
//==============================================================================
void DrawHQImage(int16_t *pImage, tRGBcolor *pPalette, uint16_t PaletteSize, uint16_t X, uint16_t Y, uint16_t width, uint16_t height, float minTemp)
{
	int cnt = 0;
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++, cnt++)
		{
			int16_t colorIdx = pImage[cnt] - (minTemp * 10);

			if (colorIdx < 0)
				colorIdx = 0;
			if (colorIdx >= PaletteSize)
				colorIdx = PaletteSize - 1;

	    	uint16_t color = RGB565(pPalette[colorIdx].r, pPalette[colorIdx].g, pPalette[colorIdx].b);
	    	dispcolor_DrawPixel((width - col - 1) + X, row + Y, color);
		}
	}
}
//==============================================================================


//==============================================================================
// Процедура отрисовки цветовой шкалы
//==============================================================================
void DrawScale(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, float minTemp, float maxTemp)
{
	getPalette(ColorScale, Height, pPaletteScale);

    for (int i = 0; i < Height; i++)
	{
		uint16_t color = RGB565(pPaletteScale[i].r, pPaletteScale[i].g, pPaletteScale[i].b);
		dispcolor_FillRect(X, Y + Height - i - 1, Width, 1, color);
	}

    // Вывод максимального значения в шкале (по горизонтали - по центру)
    int16_t TextWidth = dispcolor_getFormatStrWidth(FONTID_6X8M, "%.0f°C", maxTemp);
    dispcolor_printf(X + (Width - TextWidth) / 2, Y + 2, FONTID_6X8M, ColorScale == Rainbow || ColorScale == Rainbow2 || ColorScale == BlueRed ? WHITE : BLACK, "%.0f°C", maxTemp);
    // Вывод минимального значения в шкале (по горизонтали - по центру)
    TextWidth = dispcolor_getFormatStrWidth(FONTID_6X8M, "%.0f°C", minTemp);
    dispcolor_printf(X + (Width - TextWidth) / 2, Y + Height - 10, FONTID_6X8M, WHITE, "%.0f°C", minTemp);
}
//==============================================================================


//==============================================================================
// Процедура вывода "прицела" и значения температуры в центре термограммы заданным цветом
//==============================================================================
static void DrawCenterTempColor(uint16_t cX, uint16_t cY, float Temp, uint16_t color)
{
	uint8_t offMin = 5;		// Расстояние от центра до начала рисок
	uint8_t offMax = 10;	// Расстояние от центра до конца рисок
	uint8_t offTwin = 1;	// Расстояние между 2 параллельными рисками

	// Верхняя часть перекрестия
	dispcolor_DrawLine(cX - offTwin, cY - offMin, cX - offTwin, cY - offMax, color);
	dispcolor_DrawLine(cX + offTwin, cY - offMin, cX + offTwin, cY - offMax, color);
	// Нижняя часть перекрестия
	dispcolor_DrawLine(cX - offTwin, cY + offMin, cX - offTwin, cY + offMax, color);
	dispcolor_DrawLine(cX + offTwin, cY + offMin, cX + offTwin, cY + offMax, color);
	// Левая часть перекрестия
	dispcolor_DrawLine(cX - offMin, cY - offTwin, cX - offMax, cY - offTwin, color);
	dispcolor_DrawLine(cX - offMin, cY + offTwin, cX - offMax, cY + offTwin, color);
	// Правая часть перекрестия
	dispcolor_DrawLine(cX + offMin, cY - offTwin, cX + offMax, cY - offTwin, color);
	dispcolor_DrawLine(cX + offMin, cY + offTwin, cX + offMax, cY + offTwin, color);

	if ((Temp > -100) && (Temp < 500))
		dispcolor_printf(cX + 8, cY + 8, FONTID_6X8M, color, "%.1f°C", Temp);
}
//==============================================================================


//==============================================================================
// Процедура вывода "прицела" и значения температуры в центре термограммы белым с чёрной тенью
//==============================================================================
void DrawCenterTemp(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, float Temp)
{
	uint16_t cX = (Width >> 1) + X;
	uint16_t cY = (Height >> 1) + Y;

	// Отрисовка тени чёрным
	DrawCenterTempColor(cX + 1, cY + 1, Temp, BLACK);
	// Отрисовка белым
	DrawCenterTempColor(cX, cY, Temp, WHITE);
}
//==============================================================================


static void ThermoToImagePosition(int16_t *pX, int16_t *pY)
{
	switch (ScaleMode)
	{
	case ByNearest:
	case HQ3X_2X:
		*pX = *pX * 9;		// 32 * 9 = 288
		*pY = *pY * 9;		// 24 * 9 = 216
		break;
	case LineInterpol:
		*pX = *pX * 69 / 8;	// 31 * 9 = 279
		*pY = *pY * 69 / 8;	// 23 * 9 = 207
		break;
	}
}

static void DrawMarkers(uint16_t imageX, uint16_t imageY, sMlxData *pMlxData)
{
	uint8_t lineHalf = 4;
	int16_t x = termWidth - pMlxData->maxT_X - 1;
	int16_t y = pMlxData->maxT_Y;

	ThermoToImagePosition(&x, &y);
	x += imageX + 4;
	y += imageY + 4;

	uint16_t mainColor = WHITE;
	dispcolor_DrawLine(x + 1, y - lineHalf + 1, x + 1, y + lineHalf + 1, BLACK);
	dispcolor_DrawLine(x - lineHalf + 1, y + 1, x + lineHalf + 1, y + 1, BLACK);

	dispcolor_DrawLine(x - lineHalf + 1, y - lineHalf + 1, x + lineHalf + 1, y + lineHalf + 1, BLACK);
	dispcolor_DrawLine(x - lineHalf + 1, y + lineHalf + 1, x + lineHalf + 1, y - lineHalf + 1, BLACK);

	dispcolor_DrawLine(x, y - lineHalf, x, y + lineHalf, mainColor);
	dispcolor_DrawLine(x - lineHalf, y, x + lineHalf, y, mainColor);

	dispcolor_DrawLine(x - lineHalf, y - lineHalf, x + lineHalf, y + lineHalf, mainColor);
	dispcolor_DrawLine(x - lineHalf, y + lineHalf, x + lineHalf, y - lineHalf, mainColor);

	x = termWidth - pMlxData->minT_X - 1;
	y = pMlxData->minT_Y;

	ThermoToImagePosition(&x, &y);
	x += imageX + 4;
	y += imageY + 4;

//	mainColor = RGB565(200, 200, 255);
	dispcolor_DrawLine(x - lineHalf + 1, y - lineHalf + 1, x + lineHalf + 1, y + lineHalf + 1, BLACK);
	dispcolor_DrawLine(x - lineHalf + 1, y + lineHalf + 1, x + lineHalf + 1, y - lineHalf + 1, BLACK);
	dispcolor_DrawLine(x - lineHalf, y - lineHalf, x + lineHalf, y + lineHalf, mainColor);
	dispcolor_DrawLine(x - lineHalf, y + lineHalf, x + lineHalf, y - lineHalf, mainColor);
}


//==============================================================================
// Процедура интерполяции термограммы
//==============================================================================
void InterpolateImage(int16_t *pImage, int16_t *pHdImage)
{
	uint32_t OutIdx = 0;

	// Растягиваем точки по горизонтали 32 -> 279 (288)
	for (uint16_t row = 0; row < termHeight; row++)
	{
		for (uint16_t col = 0; col < (termWidth - 1); col++)
		{
			uint16_t ImageIdx = row * termWidth + col;
			int16_t tempStart = pImage[ImageIdx];
			int16_t tempEnd = pImage[ImageIdx + 1];

			for (uint16_t step = 0; step < iSteps; step++)
			{
				uint32_t Idx = (OutIdx + col) * iSteps + step;
				pHdImage[Idx] = (tempStart * (iSteps - step) + tempEnd * step) / iSteps;
			}
		}

		OutIdx += imageWidth;
	}

	// Растягиваем точки по вертикали 24 -> 207 (216)
	for (uint16_t col = 0; col < imageWidth; col++)
	{
		for (uint16_t row = 0; row < termHeight; row++)
		{
			int16_t tempStart = pHdImage[row * iSteps * imageWidth + col];
			int16_t tempEnd = pHdImage[(row + 1) * iSteps * imageWidth + col];

			for (uint16_t step = 1; step < iSteps; step++)
			{
				uint32_t Idx = (row * iSteps + step) * imageWidth + col;
				pHdImage[Idx] = tempStart * (iSteps - step) / iSteps + tempEnd * step / iSteps;
			}
		}
	}
}
//==============================================================================


//==============================================================================
// Процедура вывода значка батареи
//==============================================================================
void DrawBattery(uint16_t X, uint16_t Y, float capacity)
{
	// Определяем цвет делений
	uint16_t Color = GREEN;
	if (capacity < 80)
		Color = RGB565(249, 166, 2);
	if (capacity < 50)
		Color = RED;

	// Рисуем контур батарейки
	dispcolor_DrawRectangle(X, Y, X + 17, Y + 9, WHITE);
	dispcolor_DrawRectangleFilled(X + 17, Y + 2, X + 19, Y + 6, WHITE);
	// Рисуем деления
	dispcolor_DrawRectangleFilled(X + 12, Y + 2, X + 15, Y + 7, capacity < 80 ? BLACK : Color);
	dispcolor_DrawRectangleFilled(X + 7, Y + 2, X + 10, Y + 7, capacity < 50 ? BLACK : Color);
	dispcolor_DrawRectangleFilled(X + 2, Y + 2, X + 5, Y + 7, capacity < 25 ? BLACK : Color);
}
//==============================================================================


void ProcessButtonEvents(void)
{
	if (xButtonsEventQueue)
	{
		uint8_t event = No_Event;
        if (xQueueReceive(xButtonsEventQueue, &event, (TickType_t) 1))
        {
        	switch (event)
        	{
        	case ShortPress_Up:
        		FuncUp_Run();
        		break;
        	case ShortPress_Center:
        		menu_run();
        		settings_write_all();
        		dispcolor_ClearScreen();
        		Need2RedrawTitle = 1;
        		Need2RedrawPalette = 1;
        		break;
        	case ShortPress_Down:
        		FuncDown_Run();
        		break;
        	}
        }
	}
}

void ReDrawTitle(void)
{
    dispcolor_printf(2, 4, FONTID_6X8M, WHITE, "Тепловизор V%d.%d  %.1f FPS\r\n", SW_VERSION_MAJOR, SW_VERSION_MINOR, FPS_rates[FPS_Idx]);
}

void RedrawPalette(void)
{
	// Отрисовка шкалы в правой части экрана
	DrawScale(imageWidth + 2, (dispHeight - imageHeight) >> 1, dispWidth - imageWidth - 2, imageHeight, minTemp, maxTemp);
	// Генерация новой цветовой шкалы
	PaletteSteps = (uint16_t)((maxTemp - minTemp) * 10 + 1);
	getPalette(ColorScale, PaletteSteps, pPaletteImage);
}

int8_t AllocThermoImageBuffers(void)
{
	// Буферы для алгоритмов масштабирования, исходного и итогового изображения
    TermoImage16 = heap_caps_malloc((termWidth * termHeight) << 1, MALLOC_CAP_8BIT/* | MALLOC_CAP_INTERNAL*/);
    TermoStage1 = heap_caps_malloc((termWidth * 3 * termHeight * 3) << 1, MALLOC_CAP_8BIT/* | MALLOC_CAP_INTERNAL*/);
	TermoHqImage16 = heap_caps_malloc((termWidth * 9 * termHeight * 9) << 1, MALLOC_CAP_8BIT /*| MALLOC_CAP_INTERNAL*/);
    if (!TermoImage16 || !TermoStage1 || !TermoHqImage16)
    	return -1;
    return 0;
}


int8_t AllocPaletteImageBuffer(void)
{
	pPaletteImage = heap_caps_malloc(((MAX_TEMP - MIN_TEMP) * 10) << 1, MALLOC_CAP_8BIT);	// Буфер палитры изображения
	if (!pPaletteImage)
		return -1;
	return 0;
}

//==============================================================================
//
//==============================================================================
void ui_task(void* arg)
{
	Need2RedrawTitle = 1;

	dispcolor_ClearScreen();

    while (1)
    {
    	switch (ScaleMode)
    	{
    	case ByNearest:
    		imageHeight = termHeight * 9;
			imageWidth = termWidth * 9;
    		break;
    	case LineInterpol:
    		imageHeight = (termHeight - 1) * iSteps;
			imageWidth = (termWidth - 1) * iSteps;
    		break;
    	case HQ3X_2X:
    		imageHeight = termHeight * 9;
			imageWidth = termWidth * 9;
    		break;
    	}

    	int16_t imageY = (dispHeight - imageHeight) >> 1;	// Координата размещения термограммы на дисплее
		sMlxData *pMlxData = &MlxData[(MlxDataIdx + 1) & 1];
		float *ThermoImage = pMlxData->ThermoImage;

		// Формирование новой шкалы если необходимо
		if (AutoScaleMode)
		{
			minTempNew = pMlxData->minT;
			maxTempNew = pMlxData->maxT;
		}

		if (Need2RedrawTitle)
		{
			Need2RedrawTitle = 0;
			ReDrawTitle();
		}

		if (Need2RedrawPalette)
		{
			Need2RedrawPalette = 0;
			RedrawPalette();
		}

		if ((minTempNew != minTemp) || (maxTempNew != maxTemp))
        {
        	// Расширяем шкалу если диапазон получился меньше MIN_TEMPSCALE_DELTA
        	if (AutoScaleMode)
        	{
        		float Delta = maxTempNew - minTempNew;
        		if (Delta < MIN_TEMPSCALE_DELTA)
        		{
        			minTempNew -= (MIN_TEMPSCALE_DELTA - Delta) / 2;
        			maxTempNew += (MIN_TEMPSCALE_DELTA - Delta) / 2;
        		}
        	}

        	minTemp = minTempNew;
        	maxTemp = maxTempNew;
        	RedrawPalette();
        }

    	// Копируем температуры в целочисленный массив для упрощения дальнейших расчётов
    	for (uint16_t i = 0; i < 768; i++)
        	TermoImage16[i] = ThermoImage[i] * 10;

		// Масштабируем и выводим термограму
    	switch (ScaleMode)
    	{
    	case ByNearest:
  			DrawImage(TermoImage16, pPaletteImage, PaletteSteps, 0, imageY, 9, 9, minTemp);
    		break;
    	case LineInterpol:
        	InterpolateImage(TermoImage16, TermoHqImage16);
       		DrawHQImage(TermoHqImage16, pPaletteImage, PaletteSteps, 0, imageY, imageWidth, imageHeight, minTemp);
    		break;
    	case HQ3X_2X:
        	hqx_scale(3, termWidth, termHeight, TermoImage16, TermoStage1);
        	hqx_scale(3, termWidth * 3, termHeight * 3, TermoStage1, TermoHqImage16);
       		DrawHQImage(TermoHqImage16, pPaletteImage, PaletteSteps, 0, imageY, imageWidth, imageHeight, minTemp);
    		break;
    	}

    	// Маркеры MAX/MIN на термограмме
    	if (TempMarkers)
    		DrawMarkers(0, imageY, pMlxData);

    	// Выводим температуру в центре экрана
    	DrawCenterTemp(0, imageY, imageWidth, imageHeight, pMlxData->CenterTemp);

    	// Вывод минимальной и максимальной температуры в кадре
		dispcolor_printf_Bg(1, 228, FONTID_6X8M, RGB565(32, 32, 192), BLACK, "MIN=%.1f°C ", pMlxData->minT);
		dispcolor_printf_Bg(71, 228, FONTID_6X8M, RGB565(192, 32, 32), BLACK, "MAX=%.1f°C ", pMlxData->maxT);
		dispcolor_printf_Bg(142, 228, FONTID_6X8M, RGB565(160, 96, 0), BLACK, "VBAT=%.2fV ", pMlxData->VBAT);
        dispcolor_printf_Bg(217, 228, FONTID_6X8M, RGB565(0, 160, 160), BLACK, "Ta=%.1f°C ", pMlxData->Ta);
        dispcolor_printf_Bg(284, 228, FONTID_6X8M, RGB565(96, 160, 0), BLACK, "E=%.2f ", Emissivity);

		// Считаем и выводим заряд аккумулятора и его напряжение
		float capacity = pMlxData->VBAT * 125 - 400;
		if (capacity > 100)
			capacity = 100;
		if (capacity < 0)
			capacity = 0;
		DrawBattery(300, 2, capacity);

		// Обновляем дисплей из буфера кадра
    	dispcolor_Update();

    	// Обработка нажатия кнопок
		ProcessButtonEvents();
    }
}
//==============================================================================
