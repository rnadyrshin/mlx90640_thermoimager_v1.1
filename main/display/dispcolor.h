#ifndef _DISPCOLOR_H
#define _DISPCOLOR_H

#include "fonts/font.h"

// ������ �������������� �������� (������������ ��������)
#define DISPTYPE_st7789         0
#define DISPTYPE_st7735         1
#define DISPTYPE_ili9641        2

// � ����� ����� ������� ����� �������� ������ disp1color
#define DISPCOLOR_type         DISPTYPE_ili9641


#define RGB565(r, g, b)         (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

typedef union _uRGB565
{
	uint16_t value;
	struct
	{
		uint16_t b	:5;
		uint16_t g	:6;
		uint16_t r	:5;
	} rgb_color;
} uRGB565;


// ������� �����
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF


// ��������� �������������� ������� �������
void dispcolor_Init(uint16_t Width, uint16_t Height);
// �������, ������������ ������� ������� � ��������
uint16_t dispcolor_getWidth();
uint16_t dispcolor_getHeight();
// ��������� ������� ����� (����������� ������)
void dispcolor_ClearScreen(void);
// ��������� ������������� ������� �������
void dispcolor_SetBrightness(uint8_t Value);
// ��������� ���������� �������������� ������ color
void dispcolor_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
// ��������� ���������� 1 ������� �������
void dispcolor_DrawPixel(int16_t X, int16_t Y, uint16_t color);
// ��������� ���������� ���� �������
uint16_t dispcolor_GetPixel(int16_t x, int16_t y);
// ��������� ����������� ����� ������ color
void dispcolor_FillScreen(uint16_t color);
// ��������� ��������� ������� �� ������ �����
void dispcolor_Update(void);
// ��������� ������ ������ ����� �� �������
void dispcolor_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
// ��������� ������ ������������� �� �������
void dispcolor_DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
// ��������� ������ ����������� ������������� �� �������
void dispcolor_DrawRectangleFilled(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t fillcolor);
// ��������� ������ ���������� �� �������. x0 � y0 - ���������� ������ ����������
void dispcolor_DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color);
// ��������� ������ ����������� ���������� �� �������. x0 � y0 - ���������� ������ ����������
void dispcolor_DrawCircleFilled(int16_t x0, int16_t y0, int16_t radius, uint16_t fillcolor);
// ������� ������ ������ �� ������ Str �� �������. ���������� ������ ������ � ��������
int16_t dispcolor_DrawString(int16_t X, int16_t Y, uint8_t FontID, uint8_t *Str, uint16_t TextColor);
// ������� ������ ������ �� ������ Str �� �������. ���������� ������ ������ � ��������
int16_t dispcolor_DrawString_Bg(int16_t X, int16_t Y, uint8_t FontID, uint8_t *Str, uint16_t TextColor, uint16_t BgColor);
// ��������� ������� �� ������� ��������������� ������. ���������� ������ ������ � ��������
int16_t dispcolor_printf(int16_t X, int16_t Y, uint8_t FontID, uint16_t TextColor, const char *args, ...);
// ��������� ������� �� ������� ��������������� ������. ���������� ������ ������ � ��������
int16_t dispcolor_printf_Bg(int16_t X, int16_t Y, uint8_t FontID, uint16_t TextColor, uint16_t BgColor, const char *args, ...);
// ������� ���������� ������ ������ � ��������
int16_t dispcolor_getStrWidth(uint8_t FontID, char *Str);
// ������� ���������� ������ ��������������� ������ � ��������
int16_t dispcolor_getFormatStrWidth(uint8_t FontID, const char *args, ...);
// ��������� ������� ������� ����������� �� ������ � ~2 ����
void dispcolor_screenDark(void);
// ��������� �������� ����� ����� � ����� pBuff
void dispcolor_getScreenData(uint16_t *pBuff);

#endif
