#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "../display/dispcolor.h"
#include "../main.h"
#include "palette.h"


//==============================================================================
// Процедура заполняет буфер pBuff точками палитры (steps - кол-во шагов палитры)
//==============================================================================
static void getIronPalette(uint16_t steps, tRGBcolor *pBuff)
{
	if (!pBuff)
	    return;

    if (steps % 4)
    	steps = (steps / 4) * 4 + 4;

	uint16_t partSize = steps >> 2;
	tRGBcolor KeyColors[] =
	{
		{0x00, 0x00, 0x00},	// Чёрный
		{0x20, 0x00, 0x8C},	// Тёмно-синий
		{0xCC, 0x00, 0x77},	// Фиолетовый
		{0xFF, 0xD7, 0x00},	// Золотой
		{0xFF, 0xFF, 0xFF}	// Белый
	};

	for (uint8_t part = 0; part < 4; part++)
	{
	    for (uint16_t step = 0; step < partSize; step++)
		{
		    float n = (float)step / (float) (partSize - 1);

		    pBuff->r = ((float)KeyColors[part].r) * (1.0f - n) + ((float)KeyColors[part + 1].r) * n;
		    pBuff->g = ((float)KeyColors[part].g) * (1.0f - n) + ((float)KeyColors[part + 1].g) * n;
		    pBuff->b = ((float)KeyColors[part].b) * (1.0f - n) + ((float)KeyColors[part + 1].b) * n;

		    pBuff++;
		}
	}
}
//==============================================================================


//==============================================================================
// Процедура заполняет буфер pBuff точками палитры (steps - кол-во шагов палитры)
//==============================================================================
static void getRainbowPalette(uint16_t steps, tRGBcolor *pBuff)
{
	if (!pBuff)
	    return;

    if (steps % 5)
    	steps = (steps / 5) * 5 + 5;

	uint16_t partSize = steps / 5;
	tRGBcolor KeyColors[] =
	{
		{84, 0, 180},	// Фиолетовый
		{0, 97, 211},	// Синий
		{0, 145, 72},	// Зелёный
		{207, 214, 0},	// Жёлтый
		{231, 108, 0},	// Оранжевый
		{193, 19, 33}	// Красный
	};

	for (uint8_t part = 0; part < 5; part++)
	{
	    for (uint16_t step = 0; step < partSize; step++)
		{
		    float n = (float)step / (float) (partSize - 1);

		    pBuff->r = ((float)KeyColors[part].r) * (1.0f - n) + ((float)KeyColors[part + 1].r) * n;
		    pBuff->g = ((float)KeyColors[part].g) * (1.0f - n) + ((float)KeyColors[part + 1].g) * n;
		    pBuff->b = ((float)KeyColors[part].b) * (1.0f - n) + ((float)KeyColors[part + 1].b) * n;

		    pBuff++;
		}
	}
}
//==============================================================================


//==============================================================================
// Процедура заполняет буфер pBuff точками палитры (steps - кол-во шагов палитры)
//==============================================================================
static void getRainbow2Palette(uint16_t steps, tRGBcolor *pBuff)
{
	if (!pBuff)
	    return;

    if (steps % 6)
    	steps = (steps / 6) * 6 + 6;

	uint16_t partSize = steps / 6;
	tRGBcolor KeyColors[] =
	{
		{143, 0, 255},	// Фиолетовый
		{75, 0, 130},	// Фиолетовый
		{0, 0, 255},	// Синий
		{0, 255, 0},	// Зелёный
		{255, 255, 0},	// Жёлтый
		{255, 127, 0},	// Оранжевый
		{255, 0, 0}		// Красный
	};

	for (uint8_t part = 0; part < 6; part++)
	{
	    for (uint16_t step = 0; step < partSize; step++)
		{
		    float n = (float)step / (float) (partSize - 1);

		    pBuff->r = ((float)KeyColors[part].r) * (1.0f - n) + ((float)KeyColors[part + 1].r) * n;
		    pBuff->g = ((float)KeyColors[part].g) * (1.0f - n) + ((float)KeyColors[part + 1].g) * n;
		    pBuff->b = ((float)KeyColors[part].b) * (1.0f - n) + ((float)KeyColors[part + 1].b) * n;

		    pBuff++;
		}
	}
}
//==============================================================================


//==============================================================================
// Процедура заполняет буфер pBuff точками палитры (steps - кол-во шагов палитры)
//==============================================================================
static void getBlackNWhitePalette(uint16_t steps, tRGBcolor *pBuff)
{
	if (!pBuff)
	    return;

	tRGBcolor KeyColors[] =
	{
		{0x00, 0x00, 0x00},	// Чёрный
		{0xFF, 0xFF, 0xFF}	// Белый
	};

    for (uint16_t step = 0; step < steps; step++)
	{
	    float n = (float)step / (float) (steps - 1);

	    pBuff->r = ((float)KeyColors[0].r) * (1.0f - n) + ((float)KeyColors[1].r) * n;
	    pBuff->g = ((float)KeyColors[0].g) * (1.0f - n) + ((float)KeyColors[1].g) * n;
	    pBuff->b = ((float)KeyColors[0].b) * (1.0f - n) + ((float)KeyColors[1].b) * n;

	    pBuff++;
	}
}
//==============================================================================


//==============================================================================
// Процедура заполняет буфер pBuff точками палитры (steps - кол-во шагов палитры)
//==============================================================================
static void getBlueRedPalette(uint16_t steps, tRGBcolor *pBuff)
{
	if (!pBuff)
	    return;

	tRGBcolor KeyColors[] =
	{
		{0x00, 0x00, 0xFF},	// Синий
		{0xFF, 0x00, 0x00}	// Красный
	};

    for (uint16_t step = 0; step < steps; step++)
	{
	    float n = (float)step / (float) (steps - 1);

	    pBuff->r = ((float)KeyColors[0].r) * (1.0f - n) + ((float)KeyColors[1].r) * n;
	    pBuff->g = ((float)KeyColors[0].g) * (1.0f - n) + ((float)KeyColors[1].g) * n;
	    pBuff->b = ((float)KeyColors[0].b) * (1.0f - n) + ((float)KeyColors[1].b) * n;

	    pBuff++;
	}
}
//==============================================================================


//==============================================================================
// Функция возвращает указатель на массив точек палитры выбранного типа
//==============================================================================
void getPalette(eColorScale palette, uint16_t steps, tRGBcolor *pBuff)
{
	switch (palette)
	{
		case Iron:
			getIronPalette(steps, pBuff);
			break;
		case Rainbow:
			getRainbowPalette(steps, pBuff);
			break;
		case Rainbow2:
			getRainbow2Palette(steps, pBuff);
			break;
		case BlueRed:
			getBlueRedPalette(steps, pBuff);
			break;
		case BlackNWhite:
			getBlackNWhitePalette(steps, pBuff);
			break;
	}
}
//==============================================================================
