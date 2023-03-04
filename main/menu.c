#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "display/dispcolor.h"
#include "ili9341/ili9341.h"
#include "palette/palette.h"
#include "task_buttons.h"
#include "task_ui.h"
#include "settings.h"
#include "main.h"
#include "menu.h"


typedef enum
{
	Button,
	CheckBox,
	FloatValue,
	IntValue,
	ComboBox,
	PaletteBox
} eMenuItemType;

typedef struct
{
	char Str[30];
}
sComboItem;

typedef struct
{
	char Title[40];
	eMenuItemType ItemType;

	// Параметры изменяемой числовой величины (IntValue, FloatValue)
	float Min;
	float Max;
	float EditStep;
	uint8_t DigitsAfterPoint;
	char Unit[10];

	// Строки выбираемых пунктов (в ComboBox)
	sComboItem *ComboItems;
	uint8_t ComboItemsCount;

	// Изменяемое значение
	void *pValue;			// Указатель на изменяемое значение (для IntValue, FloatValue, ComboBox)

	// Action
	void (*Action) (void);	// Указатель на функцию пунктов меню, которые подразумевают действие (например, Button)
} sMenuItem;

typedef struct
{
	uint16_t startX;
	uint16_t startY;
	uint16_t width;
	char Title[30];

	sMenuItem *items;
	uint8_t itemsCount;
	uint8_t selectedItemIdx;
	uint8_t OnEdit;
} sMenu;

static sMenu Menu;
static uint8_t posY = 0;


static uint8_t exitRequest;	// Флаг необходимости закрыть меню

// Действие выхода из меню
static void menu_exit(void)
{
	exitRequest = 1;
}

static void add_menuitem(sMenuItem *pNewItem)
{
	uint8_t oldCount = Menu.itemsCount;
	uint8_t menuItemSize = sizeof(sMenuItem);

	// Выделяем новый буфер в памяти для хранения массива указателей на пункты меню
	sMenuItem *pMenuArray = heap_caps_malloc((oldCount + 1) * menuItemSize, MALLOC_CAP_8BIT);
	if (Menu.items)
	{
		// Копируем уже имеющиеся в списке указатели в новый буфер
		memcpy(pMenuArray, Menu.items, oldCount * menuItemSize);
		// Освобождаем память, занимаемую старым буфером
		heap_caps_free(Menu.items);
	}

	// Заменяем старый буфер на новый
	Menu.items = pMenuArray;
	// Добавляем новый элемент
	memcpy(&(pMenuArray[oldCount]), pNewItem, menuItemSize);
	Menu.itemsCount++;

	posY +=12;
}


int menu_build()
{
	// Сброс структуры меню
	Menu.itemsCount = 0;
	// Сборка структуры меню
	Menu.startX = 45;
	//Menu.startY = 41;

	strcpy(Menu.Title, "Настройки тепловизора");

	sMenuItem item;
	item.Action = 0;
	uint8_t idx;
	posY = 12;

	strcpy(item.Title, "Emissivity:");
	item.ItemType = FloatValue;
	item.pValue = &Emissivity;
	item.Min = EMISSIVITY_MIN;
	item.Max = EMISSIVITY_MAX;
	strcpy(item.Unit, "");
	item.EditStep = EMISSIVITY_STEP;
	item.DigitsAfterPoint = 2;
	add_menuitem(&item);

	strcpy(item.Title, "Масштабирование:");
	item.ItemType = ComboBox;
	item.ComboItemsCount = 3;
	item.ComboItems = heap_caps_malloc(item.ComboItemsCount * sizeof(sComboItem), MALLOC_CAP_8BIT);
	idx = 0;
	strcpy(item.ComboItems[idx++].Str, "По ближайшему");
	strcpy(item.ComboItems[idx++].Str, "Лин. интерпол.");
	strcpy(item.ComboItems[idx++].Str, "Алг. HQ3X (2x)");
	item.pValue = &ScaleMode;
	add_menuitem(&item);

	strcpy(item.Title, "Частота обновления:");
	item.ItemType = ComboBox;
	item.ComboItemsCount = 8;
	item.ComboItems = heap_caps_malloc(item.ComboItemsCount * sizeof(sComboItem), MALLOC_CAP_8BIT);
	idx = 0;
	strcpy(item.ComboItems[idx++].Str, "0.5 FPS");
	strcpy(item.ComboItems[idx++].Str, "1 FPS");
	strcpy(item.ComboItems[idx++].Str, "2 FPS");
	strcpy(item.ComboItems[idx++].Str, "4 FPS");
	strcpy(item.ComboItems[idx++].Str, "8 FPS");
	strcpy(item.ComboItems[idx++].Str, "16 FPS");
	strcpy(item.ComboItems[idx++].Str, "32 FPS");
	strcpy(item.ComboItems[idx++].Str, "64 FPS");
	item.pValue = &FPS_Idx;
	add_menuitem(&item);

	strcpy(item.Title, "Автонастройка шкалы:");
	item.ItemType = CheckBox;
	item.pValue = &AutoScaleMode;
	add_menuitem(&item);

	strcpy(item.Title, "Мин. темп.:");
	item.ItemType = FloatValue;
	item.pValue = &minTempNew;
	item.Min = -40;
	item.Max = 300;
	strcpy(item.Unit, " °C");
	item.EditStep = 1;
	item.DigitsAfterPoint = 0;
	add_menuitem(&item);

	strcpy(item.Title, "Макс. темп.:");
	item.ItemType = FloatValue;
	item.pValue = &maxTempNew;
	item.Min = -40;
	item.Max = 300;
	strcpy(item.Unit, " °C");
	item.EditStep = 1;
	item.DigitsAfterPoint = 0;
	add_menuitem(&item);

	strcpy(item.Title, "Маркеры MAX/MIN:");
	item.ItemType = CheckBox;
	item.pValue = &TempMarkers;
	add_menuitem(&item);

	strcpy(item.Title, "Цветовая шкала:");
	item.ItemType = PaletteBox;// ComboBox;
	item.ComboItemsCount = COLORSCALE_NUM;
	item.ComboItems = heap_caps_malloc(item.ComboItemsCount * sizeof(sComboItem), MALLOC_CAP_8BIT);
	idx = 0;
	strcpy(item.ComboItems[idx++].Str, "Iron");
	strcpy(item.ComboItems[idx++].Str, "Rainbow");
	strcpy(item.ComboItems[idx++].Str, "Rainbow2");
	strcpy(item.ComboItems[idx++].Str, "BlueRed");
	strcpy(item.ComboItems[idx++].Str, "Black&White");
	item.pValue = &ColorScale;
	add_menuitem(&item);

#ifdef PIN_BL
	strcpy(item.Title, "Яркость подсветки:");
	item.ItemType = IntValue;
	item.pValue = &LcdBrightness;
	item.Min = BRIGHTNESS_MIN;
	item.Max = BRIGHTNESS_MAX;
	strcpy(item.Unit, " %");
	item.EditStep = BRIGHTNESS_STEP;
	add_menuitem(&item);
#endif

	strcpy(item.Title, "Кнопка Вверх:");
	item.ItemType = ComboBox;
	item.ComboItemsCount = 7;
#ifdef PIN_BL
	item.ComboItemsCount += 2;
#endif
	item.ComboItems = heap_caps_malloc(item.ComboItemsCount * sizeof(sComboItem), MALLOC_CAP_8BIT);
	idx = 0;
	strcpy(item.ComboItems[idx++].Str, "Emissivity +");
	strcpy(item.ComboItems[idx++].Str, "Emissivity -");
	strcpy(item.ComboItems[idx++].Str, "След. шкала");
	strcpy(item.ComboItems[idx++].Str, "Пред. шкала");
	strcpy(item.ComboItems[idx++].Str, "Маркеры MAX/MIN");
	strcpy(item.ComboItems[idx++].Str, "Сохр. в BMP");
	strcpy(item.ComboItems[idx++].Str, "Сохр. в CSV");
#ifdef PIN_BL
	strcpy(item.ComboItems[idx++].Str, "Яркость +");
	strcpy(item.ComboItems[idx++].Str, "Яркость -");
#endif
	item.pValue = &FuncUp;
	add_menuitem(&item);

	strcpy(item.Title, "Кнопка Вниз:");
	item.ItemType = ComboBox;
	item.ComboItemsCount = 7;
#ifdef PIN_BL
	item.ComboItemsCount += 2;
#endif
	item.ComboItems = heap_caps_malloc(item.ComboItemsCount * sizeof(sComboItem), MALLOC_CAP_8BIT);
	idx = 0;
	strcpy(item.ComboItems[idx++].Str, "Emissivity +");
	strcpy(item.ComboItems[idx++].Str, "Emissivity -");
	strcpy(item.ComboItems[idx++].Str, "След. шкала");
	strcpy(item.ComboItems[idx++].Str, "Пред. шкала");
	strcpy(item.ComboItems[idx++].Str, "Маркеры MAX/MIN");
	strcpy(item.ComboItems[idx++].Str, "Сохр. в BMP");
	strcpy(item.ComboItems[idx++].Str, "Сохр. в CSV");
#ifdef PIN_BL
	strcpy(item.ComboItems[idx++].Str, "Яркость +");
	strcpy(item.ComboItems[idx++].Str, "Яркость -");
#endif
	item.pValue = &FuncDown;
	add_menuitem(&item);

	strcpy(item.Title, "Выход");
	item.ItemType = Button;
	item.Action = menu_exit;
	add_menuitem(&item);

	Menu.startY = (dispHeight - posY) / 2 - 1;

	return 0;
}


static void menu_process_events(void)
{
	if (xButtonsEventQueue)
	{
		uint8_t event = No_Event;

        while (xQueueReceive(xButtonsEventQueue, &event, (TickType_t) 1))
        {
        	sMenuItem *item = &Menu.items[Menu.selectedItemIdx];
        	uint8_t *pVal_uint8 = (uint8_t *)item->pValue;
        	int* pVal_int = (int *)item->pValue;
        	float *pVal_float = (float *)item->pValue;

        	switch (event)
        	{
        	case ShortPress_Up:
    			if (Menu.OnEdit)		// Сейчас редактируем величину в выбранном пункте меню
    			{
    				switch (item->ItemType)
    				{
    				case CheckBox:
    					*pVal_uint8 = (*pVal_uint8 + 1) & 1;
    					break;
    				case FloatValue:
    					*pVal_float += item->EditStep;
    					if (*pVal_float > item->Max)
    						*pVal_float = item->Max;
    					break;
    				case IntValue:
    					*pVal_int += item->EditStep;
    					if (*pVal_int > item->Max)
    						*pVal_int = item->Max;
    					break;
    				case PaletteBox:
    				case ComboBox:
    					if (*pVal_uint8 == item->ComboItemsCount - 1)
    						*pVal_uint8 = 0;
    					else
    						*pVal_uint8 = *pVal_uint8 + 1;
    					break;
    				default:
    					break;
    				}
    			}
    			else					// Осуществляем навигацию по пунктам меню
    			{
    				if (Menu.selectedItemIdx)
    					Menu.selectedItemIdx--;
    				else
    					Menu.selectedItemIdx = Menu.itemsCount - 1;
    			}
        		break;
        	case ShortPress_Center:
        		if (item->Action)
        			item->Action();
        		else
        		{
        			if (Menu.OnEdit)		// Сейчас редактируем величину в выбранном пункте меню
        				Menu.OnEdit = 0;	// Подтверждаем и выходим из редактирования
        			else
        				Menu.OnEdit = 1;	// Начинаем редактирование
        		}
        		break;
        	case ShortPress_Down:
    			if (Menu.OnEdit)		// Сейчас редактируем величину в выбранном пункте меню
    			{
    				switch (item->ItemType)
    				{
    				case CheckBox:
    					*pVal_uint8 = (*pVal_uint8 + 1) & 1;
    					break;
    				case FloatValue:
    					*pVal_float -= item->EditStep;
    					if (*pVal_float < item->Min)
    						*pVal_float = item->Min;
    					break;
    				case IntValue:
    					*pVal_int -= item->EditStep;
    					if (*pVal_int < item->Min)
    						*pVal_int = item->Min;
    					break;
    				case PaletteBox:
    				case ComboBox:
    					if (*pVal_uint8 == 0)
    						*pVal_uint8 = item->ComboItemsCount - 1;
    					else
    						*pVal_uint8 = *pVal_uint8 - 1;
    					break;
    				default:
    					break;
    				}
    			}
    			else					// Осуществляем навигацию по пунктам меню
    			{
    				if (Menu.selectedItemIdx < Menu.itemsCount - 1)
    					Menu.selectedItemIdx++;
    				else
    					Menu.selectedItemIdx = 0;
    			}
        		break;
        	}
        }
	}
}

#define ItemFont			FONTID_6X8M
#define MenuScaleLen		60
static tRGBcolor pPaletteMenu[MenuScaleLen];

static int menu_render(void)
{
	uint16_t startX = Menu.startX;
	uint16_t startY = Menu.startY;
	uint16_t endX = dispWidth - startX;
	uint16_t endY = dispHeight - startY;

	if (exitRequest)
		return 1;

	menu_process_events();

	dispcolor_DrawRectangleFilled(startX, startY, endX, endY, BLACK);

	// Вывод заголовка меню
	int yPos = 0;
	dispcolor_DrawRectangleFilled(startX + 2, startY + yPos + 2, endX - 2, startY + yPos + 12, WHITE); //RGB565(20, 200, 20));
	dispcolor_printf(100, startY + yPos + 4, ItemFont, BLACK, Menu.Title);
	yPos += 12;

	// Вывод пунктов меню
	if (Menu.items)
	{
		for (int i = 0; i < Menu.itemsCount; i++)
		{
			int16_t textWidth;
			uint16_t mainColorTitle, bgColorTitle, mainColorValue, bgColorValue;
			sMenuItem *item = &Menu.items[i];

			if (Menu.OnEdit)
			{
				mainColorTitle = WHITE;
				bgColorTitle = BLACK;
				mainColorValue = i == Menu.selectedItemIdx ? BLACK : WHITE;
				bgColorValue = i == Menu.selectedItemIdx ? RGB565(10, 200, 100) : BLACK;
				dispcolor_DrawRectangleFilled(endX - 100, startY + yPos + 2, endX - 2, startY + yPos + 12, bgColorValue);
				dispcolor_printf(startX + 7, startY + yPos + 4, ItemFont, mainColorTitle, item->Title);
			}
			else
			{
				mainColorTitle = i == Menu.selectedItemIdx ? BLACK : WHITE;
				bgColorTitle = i == Menu.selectedItemIdx ? RGB565(10, 200, 100) : BLACK;
				mainColorValue = WHITE;
				bgColorValue = BLACK;
				dispcolor_DrawRectangleFilled(startX + 2, startY + yPos + 2, endX - 100, startY + yPos + 12, bgColorTitle);
				dispcolor_printf(startX + 7, startY + yPos + 4, ItemFont, mainColorTitle, item->Title);
			}

			uint16_t color;

			switch (item->ItemType)
			{
			case CheckBox:
				if (*((uint8_t *)(item->pValue)))
					dispcolor_printf(endX - dispcolor_getStrWidth(ItemFont, "Да") - 5, startY + yPos + 4, ItemFont, mainColorValue, "Да");
				else
					dispcolor_printf(endX - dispcolor_getStrWidth(ItemFont, "Нет") - 5, startY + yPos + 4, ItemFont, mainColorValue, "Нет");
				break;
			case ComboBox:
				if (item->ComboItemsCount)
				{
					char *pStr = item->ComboItems[*((uint8_t *) item->pValue)].Str;
					textWidth = dispcolor_getStrWidth(ItemFont, pStr);
					dispcolor_printf(endX - textWidth - 5, startY + yPos + 4, FONTID_6X8M, mainColorValue, pStr);
				}
				break;
			case PaletteBox:
				getPalette(ColorScale, MenuScaleLen, pPaletteMenu);
				for (int i = 0; i < MenuScaleLen; i++)
				{
					color = RGB565(pPaletteMenu[i].r, pPaletteMenu[i].g, pPaletteMenu[i].b);
					dispcolor_FillRect(endX - MenuScaleLen + i - 6, startY + yPos + 3, 1, 8, color);
				}
				dispcolor_DrawRectangle(endX - MenuScaleLen - 6, startY + yPos + 3, endX - 6, startY + yPos + 11, WHITE);
				break;
			case FloatValue:
				switch (item->DigitsAfterPoint)
				{
				case 0:
					textWidth = dispcolor_getFormatStrWidth(ItemFont, "%.0f%s", *((float *)(item->pValue)), item->Unit);
					dispcolor_printf(endX - textWidth - 5, startY + yPos + 4, ItemFont, mainColorValue, "%.0f%s", *((float *)(item->pValue)), item->Unit);
					break;
				case 1:
					textWidth = dispcolor_getFormatStrWidth(ItemFont, "%.1f%s", *((float *)(item->pValue)), item->Unit);
					dispcolor_printf(endX - textWidth - 5, startY + yPos + 4, ItemFont, mainColorValue, "%.1f%s", *((float *)(item->pValue)), item->Unit);
					break;
				case 2:
					textWidth = dispcolor_getFormatStrWidth(ItemFont, "%.2f%s", *((float *)(item->pValue)), item->Unit);
					dispcolor_printf(endX - textWidth - 5, startY + yPos + 4, ItemFont, mainColorValue, "%.2f%s", *((float *)(item->pValue)), item->Unit);
					break;
				case 3:
					textWidth = dispcolor_getFormatStrWidth(ItemFont, "%.3f%s", *((float *)(item->pValue)), item->Unit);
					dispcolor_printf(endX - textWidth - 5, startY + yPos + 4, ItemFont, mainColorValue, "%.3f%s", *((float *)(item->pValue)), item->Unit);
					break;
				}

				break;
			case IntValue:
				textWidth = dispcolor_getFormatStrWidth(ItemFont, "%d%s", *((int *)(item->pValue)), item->Unit);
				dispcolor_printf(endX - textWidth - 5, startY + yPos + 4, ItemFont, mainColorValue, "%d%s", *((int *)(item->pValue)), item->Unit);
				break;
			default:
				break;
			}

			yPos += 12;
		}
	}

	dispcolor_Update();

	return 0;
}


int menu_run()
{
	exitRequest = 0;
	menu_build();

	dispcolor_screenDark();

	while (1)
	{
		if (menu_render())
			break;

		if (LcdBrightnessOld != LcdBrightness)
		{
			dispcolor_SetBrightness(LcdBrightness);
			LcdBrightnessOld = LcdBrightness;
		}

		vTaskDelay(50 / portTICK_RATE_MS);
	}

	return 0;
}
