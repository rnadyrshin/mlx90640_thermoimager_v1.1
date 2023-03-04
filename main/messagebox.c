#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "display/dispcolor.h"
#include "messagebox.h"


//==============================================================================
// Процедура выводит сообщение по центру экрана
//==============================================================================
void message_show(uint16_t width, uint8_t ItemFont, char *pTitle, char *pMessage, uint16_t lineColor, uint8_t darkBg, uint16_t delay_ms)
{
	// Вычисляем необходимую ширину окна
	uint16_t titleWidth = dispcolor_getStrWidth(ItemFont, pTitle);
	uint16_t messageWidth = dispcolor_getStrWidth(ItemFont, pMessage);
	uint16_t windowWidth = width ? width : titleWidth > messageWidth ? titleWidth + 20 : messageWidth + 10;
	uint16_t windowHeight = 40;

	// Считаем координаты окна
	uint16_t dispWidth = dispcolor_getWidth();
	uint16_t startX = (dispWidth - windowWidth) / 2;
	uint16_t startY = 100;
	uint16_t endX = dispWidth - startX;
	uint16_t endY = startY + windowHeight;

	// Затемняем фон
	if (darkBg)
		dispcolor_screenDark();
	// Заливаем окно чёрным
	dispcolor_DrawRectangleFilled(startX, startY, endX, endY, BLACK);

	// Выводим элементы окна
	uint16_t yPos = 0;
	// Заголовок
	dispcolor_DrawRectangleFilled(startX + 2, startY + yPos + 2, endX - 2, startY + yPos + 12, WHITE);
	dispcolor_printf(startX + (windowWidth - titleWidth) / 2, startY + yPos + 4, FONTID_6X8M, BLACK, pTitle);
	yPos += 16;
	// Текст сообщения
	dispcolor_printf(startX + 5, startY + yPos + 4, FONTID_6X8M, WHITE, pMessage);
	yPos += 16;
	// Цветная полоска внизу
	dispcolor_DrawRectangleFilled(startX + 2, startY + yPos + 2, endX - 2, startY + yPos + 5, lineColor);

	dispcolor_Update();
	vTaskDelay(delay_ms / portTICK_RATE_MS);
}
//==============================================================================


//==============================================================================
// Процедура выводит окно с сообщением и цветным индикатором хода выполнения процесса
//==============================================================================
void progress_show(uint16_t width, uint8_t ItemFont, char *pTitle, char *pMessage, uint16_t progressColor, uint16_t progressValue, uint16_t progressMax)
{
	// Вычисляем необходимую ширину окна
	uint16_t titleWidth = dispcolor_getStrWidth(ItemFont, pTitle);
	uint16_t messageWidth = dispcolor_getStrWidth(ItemFont, pMessage);
	uint16_t windowWidth = width ? width : titleWidth > messageWidth ? titleWidth + 20 : messageWidth + 10;
	uint16_t windowHeight = 40;

	// Считаем координаты окна
	uint16_t dispWidth = dispcolor_getWidth();
	uint16_t startX = (dispWidth - windowWidth) / 2;
	uint16_t startY = 100;
	uint16_t endX = dispWidth - startX;
	uint16_t endY = startY + windowHeight;

	// Заливаем окно чёрным
	dispcolor_DrawRectangleFilled(startX, startY, endX, endY, BLACK);

	// Выводим элементы окна
	uint16_t yPos = 0;
	// Заголовок
	dispcolor_DrawRectangleFilled(startX + 2, startY + yPos + 2, endX - 2, startY + yPos + 12, WHITE);
	dispcolor_printf(startX + (windowWidth - titleWidth) / 2, startY + yPos + 4, FONTID_6X8M, BLACK, pTitle);
	yPos += 16;
	// Текст сообщения
	dispcolor_printf(startX + 5, startY + yPos + 4, FONTID_6X8M, WHITE, pMessage);
	yPos += 16;
	// Цветная полоска внизу
	if (progressMax)
	{
		uint32_t endLineX = windowWidth - 4;
		endLineX = endLineX * progressValue / progressMax;
		dispcolor_DrawRectangleFilled(startX + 2, startY + yPos + 2, startX + endLineX, startY + yPos + 5, progressColor);
	}

	dispcolor_Update();
}
//==============================================================================

//==============================================================================
// Процедура выводит окно с сообщением
//==============================================================================
void progress_start_show(uint16_t width, uint8_t ItemFont, char *pTitle, char *pMessage, uint16_t progressColor, uint16_t progressValue, uint16_t progressMax)
{
	// Затемняем фон
	dispcolor_screenDark();
	progress_show(width, ItemFont, pTitle, pMessage, progressColor, progressValue, progressMax);
}
//==============================================================================

