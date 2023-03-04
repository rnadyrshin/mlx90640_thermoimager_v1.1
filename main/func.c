#include "display/dispcolor.h"
#include "save/save.h"
#include "settings.h"
#include "task_ui.h"
#include "func.h"


static void Func_Emissivity_Plus(void)
{
	if (Emissivity < EMISSIVITY_MAX)
		Emissivity += EMISSIVITY_STEP;

    setting_write("Emissivity", float32, &Emissivity);
    settings_commit();
}

static void Func_Emissivity_Minus(void)
{
	if (Emissivity > EMISSIVITY_MIN)
		Emissivity -= EMISSIVITY_STEP;

    setting_write("Emissivity", float32, &Emissivity);
    settings_commit();
}

static void Func_Scale_Next(void)
{
	if (++ColorScale == COLORSCALE_NUM)
		ColorScale = 0;

	setting_write("ColorScale", uint8, &ColorScale);
    settings_commit();
    Need2RedrawPalette = 1;
}

static void Func_Scale_Prev(void)
{
	if (!ColorScale)
		ColorScale = COLORSCALE_NUM - 1;
	else
		ColorScale--;

	setting_write("ColorScale", uint8, &ColorScale);
    settings_commit();
    Need2RedrawPalette = 1;
}

static void Func_Brightness_Plus(void)
{
	if (LcdBrightness < BRIGHTNESS_MAX)
		LcdBrightness += BRIGHTNESS_STEP;
	dispcolor_SetBrightness(LcdBrightness);
	LcdBrightnessOld = LcdBrightness;

    setting_write("LcdBrightness", int32, &LcdBrightness);
    settings_commit();
}

static void Func_Brightness_Minus(void)
{
	if (LcdBrightness > BRIGHTNESS_MIN)
		LcdBrightness -= BRIGHTNESS_STEP;
	dispcolor_SetBrightness(LcdBrightness);
	LcdBrightnessOld = LcdBrightness;

    setting_write("LcdBrightness", int32, &LcdBrightness);
    settings_commit();
}


static void Func_TempMarkers(void)
{
	TempMarkers = (TempMarkers + 1) & 1;

    setting_write("TempMarkers", uint8, &TempMarkers);
    settings_commit();
}

//==============================================================================
// Процедура, вызываемая по нажатию на кнопку Вверх
//==============================================================================
void FuncUp_Run(void)
{
	switch (FuncUp)
	{
	case Emissivity_Plus:
		Func_Emissivity_Plus();
		break;
	case Emissivity_Minus:
		Func_Emissivity_Minus();
		break;
	case Scale_Next:
		Func_Scale_Next();
		break;
	case Scale_Prev:
		Func_Scale_Prev();
		break;
	case Brightness_Plus:
		Func_Brightness_Plus();
		break;
	case Brightness_Minus:
		Func_Brightness_Minus();
		break;
	case Markers_OnOff:
		Func_TempMarkers();
		break;
	case Save_BMP16:
		save_ImageBMP_15bit();
		Need2RedrawTitle = 1;
		Need2RedrawPalette = 1;
		break;
	case Save_CSV:
		save_ImageCSV();
		Need2RedrawTitle = 1;
		Need2RedrawPalette = 1;
		break;
	}
}
//==============================================================================


//==============================================================================
// Процедура, вызываемая по нажатию на кнопку Вниз
//==============================================================================
void FuncDown_Run(void)
{
	switch (FuncDown)
	{
	case Emissivity_Plus:
		Func_Emissivity_Plus();
		break;
	case Emissivity_Minus:
		Func_Emissivity_Minus();
		break;
	case Scale_Next:
		Func_Scale_Next();
		break;
	case Scale_Prev:
		Func_Scale_Prev();
		break;
	case Brightness_Plus:
		Func_Brightness_Plus();
		break;
	case Brightness_Minus:
		Func_Brightness_Minus();
		break;
	case Markers_OnOff:
		Func_TempMarkers();
		break;
	case Save_BMP16:
		save_ImageBMP_15bit();
		Need2RedrawTitle = 1;
		Need2RedrawPalette = 1;
		break;
	case Save_CSV:
		save_ImageCSV();
		Need2RedrawTitle = 1;
		Need2RedrawPalette = 1;
		break;
	}
}
//==============================================================================

