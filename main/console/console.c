#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
//#include "ili9341/ili9341.h"
#include "../display/dispcolor.h"
#include "../display/fonts/font.h"
#include "console.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>


#define LineHeight	10
static uint8_t ConsoleLine = 0;


//==============================================================================
// Процедура делает паузу в выводе консоли
//==============================================================================
void console_pause(uint32_t timeMs)
{
    dispcolor_Update();
    vTaskDelay(timeMs / portTICK_RATE_MS);
}
//==============================================================================


//==============================================================================
// Процедура добавляет новую строку в консоль
//==============================================================================
void console_printf(eConsoleMsgType msgType, const char *args, ...)
{
	char StrBuff[256];
	uint16_t TextColor = WHITE;

	switch (msgType)
	{
	case MsgInfo:
		TextColor = GREEN;
		break;
	case MsgWarning:
		TextColor = RGB565(249, 166, 2);
		break;
	case MsgError:
		TextColor = RED;
		break;
	}

	va_list ap;
	va_start(ap, args);
	vsnprintf(StrBuff, sizeof(StrBuff), args, ap);
	va_end(ap);

	dispcolor_DrawString(0, ConsoleLine * LineHeight, FONTID_6X8M, (uint8_t *)StrBuff, TextColor);
    printf("%s", StrBuff);

    if (msgType != MsgInfo)
		console_pause(500);

	if (++ConsoleLine > 240 / LineHeight)
    {
		ConsoleLine = 0;
		console_pause(300);
        dispcolor_ClearScreen();
    }
    dispcolor_Update();
}
//==============================================================================


//==============================================================================
// Процедура выводит сообщение о перезагрузке в консоль и перезагружает esp32
//==============================================================================
void FatalError()
{
	console_printf(MsgError, "Перезагрузка через 5 секунд...\r\n");
	vTaskDelay(5000 / portTICK_RATE_MS);

    fflush(stdout);
    esp_restart();
}
//==============================================================================


//==============================================================================
// Процедура выводит сообщение об ошибке и о перезагрузке в консоль и перезагружает esp32
//==============================================================================
void FatalErrorMsg(const char *args, ...)
{
	char StrBuff[256];

	va_list ap;
	va_start(ap, args);
	vsnprintf(StrBuff, sizeof(StrBuff), args, ap);
	va_end(ap);

	console_printf(MsgError, "%s\r\n", StrBuff);
	FatalError();
}
//==============================================================================
