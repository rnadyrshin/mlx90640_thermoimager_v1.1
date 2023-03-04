//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Это программное обеспечение распространяется свободно. Вы можете размещать
// его на вашем сайте, но не забудьте указать ссылку на мой YouTube-канал
// "Электроника в объектике" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Автор: Надыршин Руслан / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#include "esp_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "dispcolor.h"
#include "fonts\font.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



#if (DISPCOLOR_type == DISPTYPE_st7789)
  #include <st77xx.h>
  #include <st7789.h>
#elif (DISPCOLOR_type == DISPTYPE_st7735)
  #include <st77xx.h>
  #include <st7735.h>
#elif (DISPCOLOR_type == DISPTYPE_ili9641)
  #include "..\ili9341\ili9341.h"
#else
  #error Работа c выбранным типом дисплея не поддерживается, попробуйте обновить модуль disp1color
#endif


static uint16_t _width, _height;


//==============================================================================
// Процедура смены местами 2 значений int16_t
//==============================================================================
static void SwapInt16Values(int16_t *pValue1, int16_t *pValue2)
{
  int16_t TempValue = *pValue1;
  *pValue1 = *pValue2;
  *pValue2 = TempValue;
}
//==============================================================================


//==============================================================================
// Процедура инициализирует цветной дисплей
//==============================================================================
void dispcolor_Init(uint16_t Width, uint16_t Height)
{
	_width = Width;
	_height = Height;

  // Инициализация дисплея
#if (DISPCOLOR_type == DISPTYPE_st7789)
  // Инициализация дисплея
  ST7789_Init(Width, Height);
#elif (DISPCOLOR_type == DISPTYPE_st7735)
  // Инициализация дисплея
  ST7735_Init(Width, Height);
#elif (DISPCOLOR_type == DISPTYPE_ili9641)
  ili9341_init(Width, Height);
#endif
  // Очистка дисплея
  dispcolor_ClearScreen();
}
//==============================================================================

//==============================================================================
// Функции, возвращающие размеры дисплея в пикселях
//==============================================================================
uint16_t dispcolor_getWidth()
{
	return _width;
}
uint16_t dispcolor_getHeight()
{
	return _height;
}
//==============================================================================


//==============================================================================
// Процедура устанавливает яркость дисплея
//==============================================================================
void dispcolor_SetBrightness(uint8_t Value)
{
  if (Value > 100)
    Value = 100;

#if (DISPCOLOR_type == DISPTYPE_st7789)
  st77xx_SetBL(Value);
#elif (DISPCOLOR_type == DISPTYPE_st7735)
  st77xx_SetBL(Value);
#elif (DISPCOLOR_type == DISPTYPE_ili9641)
  ili9341_SetBL(Value);
#endif
}
//==============================================================================


//==============================================================================
// Процедура окрашивает 1 пиксель дисплея
//==============================================================================
void dispcolor_DrawPixel(int16_t x, int16_t y, uint16_t color)
{
#if (DISPCOLOR_type == DISPTYPE_st7789)
  ST77xx_DrawPixel(x, y, color);
#elif (DISPCOLOR_type == DISPTYPE_st7735)
  ST77xx_DrawPixel(x, y, color);
#elif (DISPCOLOR_type == DISPTYPE_ili9641)
  ili9341_DrawPixel(x, y, color);
#endif
}
//==============================================================================


//==============================================================================
// Процедура возвращает цвет пикселя
//==============================================================================
uint16_t dispcolor_GetPixel(int16_t x, int16_t y)
{
#if (DISPCOLOR_type == DISPTYPE_ili9641)
  return ili9341_GetPixel(x, y);
#endif
}
//==============================================================================


//==============================================================================
// Процедура заполнения прямоугольника цветом color
//==============================================================================
void dispcolor_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
#if (DISPCOLOR_type == DISPTYPE_st7789)
  ST77xx_FillRect(x, y, w, h, color);
#elif (DISPCOLOR_type == DISPTYPE_st7735)
  ST77xx_FillRect(x, y, w, h, color);
#elif (DISPCOLOR_type == DISPTYPE_ili9641)
  ili9341_FillRect(x, y, w, h, color);
#endif
}
//==============================================================================


//==============================================================================
// Процедура обновляет дисплей из буфера кадра
//==============================================================================
void dispcolor_Update(void)
{
#if (DISPCOLOR_type == DISPTYPE_ili9641)
#if (ILI9341_MODE == ILI9341_BUFFER_MODE)
  ili9341_update();
#endif
#endif
}
//==============================================================================


//==============================================================================
// Процедура закрашивает экран цветом color
//==============================================================================
void dispcolor_FillScreen(uint16_t color)
{
  dispcolor_FillRect(0, 0, _width, _height, color);
  dispcolor_Update();
}
//==============================================================================


//==============================================================================
// Процедура очищает экран (закрашивает чёрным)
//==============================================================================
void dispcolor_ClearScreen(void)
{
  dispcolor_FillScreen(BLACK);
}
//==============================================================================


//==============================================================================
// Процедура рисует прямую линию на дисплее медленным методом (попиксельно)
//==============================================================================
static void dispcolor_DrawLine_Slow(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  const int16_t deltaX = abs(x2 - x1);
  const int16_t deltaY = abs(y2 - y1);
  const int16_t signX = x1 < x2 ? 1 : -1;
  const int16_t signY = y1 < y2 ? 1 : -1;

  int16_t error = deltaX - deltaY;

  dispcolor_DrawPixel(x2, y2, color);

  while (x1 != x2 || y1 != y2)
  {
	dispcolor_DrawPixel(x1, y1, color);
    const int16_t error2 = error * 2;

    if (error2 > -deltaY)
    {
      error -= deltaY;
      x1 += signX;
    }
    if (error2 < deltaX)
    {
      error += deltaX;
      y1 += signY;
    }
  }
}
//==============================================================================


//==============================================================================
// Процедура рисует прямую линию на дисплее
//==============================================================================
void dispcolor_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  // Вертикальная линия
  if (x1 == x2)
  {
    // Отрисовываем линию быстрым методом
    if (y1 > y2)
    	dispcolor_FillRect(x1, y2, 1, y1 - y2 + 1, color);
    else
    	dispcolor_FillRect(x1, y1, 1, y2 - y1 + 1, color);
    return;
  }

  // Горизонтальная линия
  if (y1 == y2)
  {
    // Отрисовываем линию быстрым методом
    if (x1 > x2)
    	dispcolor_FillRect(x2, y1, x1 - x2 + 1, 1, color);
    else
    	dispcolor_FillRect(x1, y1, x2 - x1 + 1, 1, color);
    return;
  }

  // Отрисовываем линию попиксельно
  dispcolor_DrawLine_Slow(x1, y1, x2, y2, color);
}
//==============================================================================


//==============================================================================
// Процедура рисует прямоугольник на дисплее
//==============================================================================
void dispcolor_DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  dispcolor_DrawLine(x1, y1, x1, y2, color);
  dispcolor_DrawLine(x2, y1, x2, y2, color);
  dispcolor_DrawLine(x1, y1, x2, y1, color);
  dispcolor_DrawLine(x1, y2, x2, y2, color);
}
//==============================================================================


//==============================================================================
// Процедура рисует закрашенный прямоугольник на дисплее
//==============================================================================
void dispcolor_DrawRectangleFilled(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t fillcolor)
{
  if (x1 > x2)
    SwapInt16Values(&x1, &x2);
  if (y1 > y2)
    SwapInt16Values(&y1, &y2);

  dispcolor_FillRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1, fillcolor);
}
//==============================================================================


//==============================================================================
// Процедура рисует окружность на дисплее. x0 и y0 - координаты центра окружности
//==============================================================================
void dispcolor_DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color)
{
  int x = 0;
  int y = radius;
  int delta = 1 - 2 * radius;
  int error = 0;

  while (y >= 0)
  {
	dispcolor_DrawPixel(x0 + x, y0 + y, color);
	dispcolor_DrawPixel(x0 + x, y0 - y, color);
	dispcolor_DrawPixel(x0 - x, y0 + y, color);
	dispcolor_DrawPixel(x0 - x, y0 - y, color);
    error = 2 * (delta + y) - 1;

    if (delta < 0 && error <= 0)
    {
      ++x;
      delta += 2 * x + 1;
      continue;
    }

    error = 2 * (delta - x) - 1;

    if (delta > 0 && error > 0)
    {
      --y;
      delta += 1 - 2 * y;
      continue;
    }

    ++x;
    delta += 2 * (x - y);
    --y;
  }
}
//==============================================================================


//==============================================================================
// Процедура рисует закрашенную окружность на дисплее. x0 и y0 - координаты центра окружности
//==============================================================================
void dispcolor_DrawCircleFilled(int16_t x0, int16_t y0, int16_t radius, uint16_t fillcolor)
{
  int x = 0;
  int y = radius;
  int delta = 1 - 2 * radius;
  int error = 0;

  while (y >= 0)
  {
    dispcolor_DrawLine(x0 + x, y0 - y, x0 + x, y0 + y, fillcolor);
    dispcolor_DrawLine(x0 - x, y0 - y, x0 - x, y0 + y, fillcolor);
    error = 2 * (delta + y) - 1;

    if (delta < 0 && error <= 0)
    {
      ++x;
      delta += 2 * x + 1;
      continue;
    }

    error = 2 * (delta - x) - 1;

    if (delta > 0 && error > 0)
    {
      --y;
      delta += 1 - 2 * y;
      continue;
    }

    ++x;
    delta += 2 * (x - y);
    --y;
  }
}
//==============================================================================


//==============================================================================
// Функция вывода символа Char на дисплей. Возвращает ширину выведенного символа
//==============================================================================
static uint8_t dispcolor_DrawChar_General(int16_t X, int16_t Y, uint8_t FontID,
                                          uint8_t Char, uint16_t TextColor,
                                          uint16_t BgColor, uint8_t TransparentBg)
{
  // Указатель на подтабличку конкретного символа шрифта
  uint8_t *pCharTable = font_GetFontStruct(FontID, Char);
  uint8_t CharWidth = font_GetCharWidth(pCharTable);    // Ширина символа
  uint8_t CharHeight = font_GetCharHeight(pCharTable);  // Высота символа
  pCharTable += 2;

  if (FontID == FONTID_6X8M)
  {
    for (uint8_t row = 0; row < CharHeight; row++)
    {
      for (uint8_t col = 0; col < CharWidth; col++)
      {
        if (pCharTable[row] & (1 << (7 - col)))
          dispcolor_DrawPixel(X + col, Y + row, TextColor);
        else if (!TransparentBg)
          dispcolor_DrawPixel(X + col, Y + row, BgColor);
      }
    }
  }
  else
  {
    for (uint8_t row = 0; row < CharHeight; row++)
    {
      for (uint8_t col = 0; col < CharWidth; col++)
      {
        if (col < 8)
        {
          if (pCharTable[row * 2] & (1 << (7 - col)))
            dispcolor_DrawPixel(X + col, Y + row, TextColor);
          else if (!TransparentBg)
            dispcolor_DrawPixel(X + col, Y + row, BgColor);
        }
        else
        {
          if (pCharTable[(row * 2) + 1] & (1 << (15 - col)))
            dispcolor_DrawPixel(X + col, Y + row, TextColor);
          else if (!TransparentBg)
            dispcolor_DrawPixel(X + col, Y + row, BgColor);
        }
      }
    }
  }

  return CharWidth;
}
//==============================================================================


//==============================================================================
// Функция вывода символа Char на дисплей. Возвращает ширину выведенного символа
//==============================================================================
uint8_t dispcolor_DrawChar(int16_t X, int16_t Y, uint8_t FontID, uint8_t Char,
                           uint16_t TextColor)
{
  return dispcolor_DrawChar_General(X, Y, FontID, Char, TextColor, 0, 1);
}
//==============================================================================


//==============================================================================
// Функция вывода символа Char на дисплей. Возвращает ширину выведенного символа
//==============================================================================
uint8_t dispcolor_DrawChar_Bg(int16_t X, int16_t Y, uint8_t FontID, uint8_t Char,
                              uint16_t TextColor, uint16_t BgColor)
{
  return dispcolor_DrawChar_General(X, Y, FontID, Char, TextColor, BgColor, 0);
}
//==============================================================================


//==============================================================================
// Функция вывода текста из строки Str на дисплей
//==============================================================================
static int16_t dispcolor_DrawString_General(int16_t X, int16_t Y, uint8_t FontID,
                                         uint8_t *Str, uint16_t TextColor,
                                         uint16_t BgColor, uint8_t TransparentBg)
{
  uint8_t done = 0;             // Флаг окончания вывода
  int16_t Xstart = X;           // Запоминаем куда будем переводить каретку при переходе на новую строку
  uint8_t StrHeight = 8;        // Высота символов в пикселях для перехода на слежующую строку

  // Вывод строки
  while (!done)
  {
    switch (*Str)
    {
    case '\0':  // Конец строки
      done = 1;
      break;
    case '\n':  // Переход на следующую строку
      Y += StrHeight;
      break;
    case '\r':  // Переход в начало строки
      X = Xstart;
      break;
    default:    // Отображаемый символ
      if (TransparentBg)
        X += dispcolor_DrawChar(X, Y, FontID, *Str, TextColor);
      else
        X += dispcolor_DrawChar_Bg(X, Y, FontID, *Str, TextColor, BgColor);

      StrHeight = font_GetCharHeight(font_GetFontStruct(FontID, *Str));
      break;
    }
    Str++;
  }
  return X;
}
//==============================================================================


//==============================================================================
// Функция вывода текста из строки Str на дисплей
//==============================================================================
int16_t dispcolor_DrawString(int16_t X, int16_t Y, uint8_t FontID, uint8_t *Str, uint16_t TextColor)
{
  return dispcolor_DrawString_General(X, Y, FontID,  Str, TextColor, 0, 1);
}
//==============================================================================


//==============================================================================
// Функция вывода текста из строки Str на дисплей
//==============================================================================
int16_t dispcolor_DrawString_Bg(int16_t X, int16_t Y, uint8_t FontID, uint8_t *Str, uint16_t TextColor, uint16_t BgColor)
{
  return dispcolor_DrawString_General(X, Y, FontID,  Str, TextColor, BgColor, 0);
}
//==============================================================================


//==============================================================================
// Функция выводит на дисплей форматированную строку
//==============================================================================
int16_t dispcolor_printf(int16_t X, int16_t Y, uint8_t FontID, uint16_t TextColor, const char *args, ...)
{
  char StrBuff[256];

  va_list ap;
  va_start(ap, args);
  vsnprintf(StrBuff, sizeof(StrBuff), args, ap);
  va_end(ap);

  return dispcolor_DrawString(X, Y, FontID, (uint8_t *)StrBuff, TextColor);
}
//==============================================================================


//==============================================================================
// Функция выводит на дисплей форматированную строку
//==============================================================================
int16_t dispcolor_printf_Bg(int16_t X, int16_t Y, uint8_t FontID, uint16_t TextColor, uint16_t BgColor, const char *args, ...)
{
  char StrBuff[256];

  va_list ap;
  va_start(ap, args);
  vsnprintf(StrBuff, sizeof(StrBuff), args, ap);
  va_end(ap);

  return dispcolor_DrawString_Bg(X, Y, FontID, (uint8_t *)StrBuff, TextColor, BgColor);
}
//==============================================================================


//==============================================================================
// Функция возвращает ширину строки в пикселях
//==============================================================================
int16_t dispcolor_getStrWidth(uint8_t FontID, char *Str)
{
	uint8_t done = 0;       // Флаг окончания вывода
	int16_t StrWidth = 0;  	// Ширина строки в пикселях

	// Вывод строки
	while (!done)
	{
		switch (*Str)
	    {
	    case '\0':  // Конец строки
	    	done = 1;
	      	break;
	    case '\n':  // Переход на следующую строку
	    case '\r':  // Переход в начало строки
	    	break;
	    default:    // Отображаемый символ
	    	StrWidth += font_GetCharWidth(font_GetFontStruct(FontID, *Str));
	    	break;
	    }
		Str++;
	}

	return StrWidth;
}
//==============================================================================


//==============================================================================
// Функция возвращает ширину форматированной строки в пикселях
//==============================================================================
int16_t dispcolor_getFormatStrWidth(uint8_t FontID, const char *args, ...)
{
  char StrBuff[256];

  va_list ap;
  va_start(ap, args);
  vsnprintf(StrBuff, sizeof(StrBuff), args, ap);
  va_end(ap);

  return dispcolor_getStrWidth(FontID, StrBuff);
}
//==============================================================================


//==============================================================================
// Процедура снижает яркость изображения на экране в ~2 раза
//==============================================================================
void dispcolor_screenDark(void)
{
	for (uint16_t y = 0; y < _height; y++)
	{
		for (uint16_t x = 0; x < _width; x++)
		{
			uRGB565 color;
			color.value = dispcolor_GetPixel(x, y);
			color.rgb_color.r >>= 1;
			color.rgb_color.g >>= 1;
			color.rgb_color.b >>= 1;
			dispcolor_DrawPixel(x, y, color.value);
		}
	}
}
//==============================================================================


//==============================================================================
// Процедура копирует буфер кадра в буфер pBuff
//==============================================================================
void dispcolor_getScreenData(uint16_t *pBuff)
{
#if (DISPCOLOR_type == DISPTYPE_ili9641)
	ili9341_getScreenData(pBuff);
#endif
}
//==============================================================================
