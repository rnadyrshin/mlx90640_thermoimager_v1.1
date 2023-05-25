#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
//#include <esp_spi_flash.h>
//#include <esp_spiram.h>
#include <esp32/spiram.h>
#include "../console/console.h"
#include "../display/dispcolor.h"
#include "../messagebox.h"
#include "../main.h"
#include "../settings.h"
#include "../task_mlx.h"
#include "sd.h"
#include <dirent.h>
#include "save.h"


#define WORD	uint16_t
#define DWORD	uint32_t
#define LONG	int32_t


static int16_t GetStringF(char *pOut, const char *args, ...)
{
	char StrBuff[32];

	va_list ap;
	va_start(ap, args);
	vsnprintf(StrBuff, sizeof(StrBuff), args, ap);
	va_end(ap);

	strcpy(pOut, StrBuff);
	return strlen (StrBuff);
}

static int32_t GetLastIndex(char *pExtensionStr)
{
	int32_t maxFileIndex = 0;
    DIR *dr = opendir("/sdcard");
    if (dr == NULL)	// �� ������� ������� �����
        return -1;

    // ����� �� ������ BMP-����� � ������������ �������� � �����
	struct dirent *de;  // Pointer for directory entry
    while ((de = readdir(dr)) != NULL)
    {
    	if (de->d_type == 1 && strstr(de->d_name, pExtensionStr))	// ������������� ������ ����� � ����������� pExtensionStr
    	{
   			uint32_t FileIndex = atoi(de->d_name);
   			if (FileIndex > maxFileIndex)
   				maxFileIndex = FileIndex;
   			//printf("NAME=%s, TYPE=%d\n", de->d_name, de->d_type);
    	}
    }
	//printf("maxFileIndex=%d\n", maxFileIndex);
	closedir(dr);

	return maxFileIndex;
}

static void WriteBmpFileHeaderCore(FILE* f, uint8_t bitsPerPixel, uint16_t width, uint16_t height)
{
	WORD uint16;
	DWORD buff32;

	if (bitsPerPixel == 15)
		bitsPerPixel = 16;
	if (bitsPerPixel == 24)
		bitsPerPixel = 32;

	uint32_t pixelDataOffset = 14 + 12;
	uint32_t fileSize = pixelDataOffset + width * height;
	fileSize *= bitsPerPixel / 8;

	// ���������� ��������� BITMAPFILEHEADER
	// ��������� ����� "BM"
	uint16 = 0x4D42;
	fwrite(&uint16, 2, 1, f);
	// ������ �����
	buff32 = fileSize;
	fwrite(&buff32, 4, 1, f);
	// 2 ��������� �����
	uint16 = 0;
	fwrite(&uint16, 2, 1, f);
	uint16 = 0;
	fwrite(&uint16, 2, 1, f);
	// �������� ������ � ������ ������������ ������ ��������� BITMAPFILEHEADER (� ������ �����)
	buff32 = pixelDataOffset;
	fwrite(&buff32, 4, 1, f);

	// ���������� ��������� BITMAPCOREHEADER
	buff32 = 12;	// ������ ���� ��������� (���������� ������ ���������
	fwrite(&buff32, 4, 1, f);
	// ������ � ������ ������
	fwrite(&width, 2, 1, f);
	fwrite(&height, 2, 1, f);
	// ���� Planes (������ = 1 ��� BMP)
	uint16 = 1;
	fwrite(&uint16, 2, 1, f);
	// ���� BitCount
	uint16 = 32;// bitsPerPixel;
	fwrite(&uint16, 2, 1, f);
}

static void WriteBmpFileHeader3(FILE* f, uint8_t bitsPerPixel, uint16_t width, uint16_t height)
{
	WORD uint16;
	DWORD uint32;
	LONG int32;

	if (bitsPerPixel == 15)
		bitsPerPixel = 16;
	if (bitsPerPixel == 24)
		bitsPerPixel = 32;

	uint32_t pixelDataOffset = 14 + 40;
	uint32_t fileSize = pixelDataOffset + width * height;
	fileSize *= bitsPerPixel / 8;

	// ���������� ��������� BITMAPFILEHEADER
	// ��������� ����� "BM"
	uint16 = 0x4D42;
	fwrite(&uint16, 2, 1, f);
	// ������ �����
	uint32 = fileSize;
	fwrite(&uint32, 4, 1, f);
	// 2 ��������� �����
	uint16 = 0;
	fwrite(&uint16, 2, 1, f);
	uint16 = 0;
	fwrite(&uint16, 2, 1, f);
	// �������� ������ � ������ ������������ ������ ��������� BITMAPFILEHEADER (� ������ �����)
	uint32 = pixelDataOffset;
	fwrite(&uint32, 4, 1, f);

	// ���������� ��������� BITMAPCOREHEADER
	uint32 = 40;	// ������ ���� ��������� (���������� ������ ���������)
	fwrite(&uint32, 4, 1, f);
	// ���� biWidth
	int32 = width;
	fwrite(&int32, 4, 1, f);
	// ���� biHeight
	int32 = height;
	fwrite(&int32, 4, 1, f);
	// ���� Planes (������ = 1 ��� BMP)
	uint16 = 1;
	fwrite(&uint16, 2, 1, f);
	// biBitCount
	uint16 = 16;
	fwrite(&uint16, 2, 1, f);
	// biCompression
	uint32 = 0;	// ��� ����������
	fwrite(&uint32, 4, 1, f);
	// biSizeImage
	uint32 = 0;	// 0 ���� ���������� �� ������������
	fwrite(&uint32, 4, 1, f);
	// ���� biXPelsPerMeter
	int32 = 0;
	fwrite(&int32, 4, 1, f);
	// ���� biYPelsPerMeter
	int32 = 0;
	fwrite(&int32, 4, 1, f);
	// ���� biClrUsed
	uint32 = 0;
	fwrite(&uint32, 4, 1, f);
	// ���� biClrImportant
	uint32 = 0;
	fwrite(&uint32, 4, 1, f);
}

static void WriteBmpRow_15bit(FILE* f, uint16_t width, uint16_t *pBuffRgb565)
{
	for (int col = 0; col < width; col++)
	{
		uRGB565 color;
		color.value = *(pBuffRgb565++);

		WORD buff16 = color.rgb_color.b;
		buff16 |= (color.rgb_color.g >> 1) << 5;
		buff16 |= color.rgb_color.r << 10;

		fwrite(&buff16, 2, 1, f);
	}
}

static void WriteBmpRow_24bit(FILE* f, uint16_t width, uint16_t *pBuffRgb565)
{
	for (int col = 0; col < width; col++)
	{
		uRGB565 color;
		color.value = *(pBuffRgb565++);
		DWORD buff32 = color.rgb_color.b << 3;
		buff32 |= (color.rgb_color.g << 8) << 2;
		buff32 |= (color.rgb_color.r << 16) << 3;

		fwrite(&buff32, 4, 1, f);
	}
}

static void WriteCsvRow(FILE* f, uint16_t width, float *pValues)
{
	for (int col = 0; col < width; col++)
	{
		if (col == width - 1)	// ��������� �������� � ������?
			fprintf(f, "%f\r\n", pValues[col]);
		else
			fprintf(f, "%f, ", pValues[col]);
	}
}

#define SAVE_CSV_WINDOW_WIDTH		200
#define SAVE_BMP_WINDOW_WIDTH		200

//==============================================================================
// ������� ���������� ������� ����������� �� ����� SD
// (������ CSV, ����������� �������� - �������)
//==============================================================================
int save_ImageCSV(void)
{
	char fileExtension[] = ".CSV";

	// ������������� ����� �� ������ ���� ��� �� ���� ���������������� ��� ���������� ����� �������������
	sd_CardInit();

	int32_t maxFileIndex = GetLastIndex(fileExtension);
	if (maxFileIndex < 0)	// �� ������� ������� �����
    {
		message_show(SAVE_CSV_WINDOW_WIDTH, FONTID_6X8M, "������ ��� ����������", "������ ������� � ����� SD", RED, 1, 1000);
	    return -1;
    }

	// �������� ��������� ����� � ������ ��� �������� ��������
	float *pValues = heap_caps_malloc((termWidth * termHeight) << 2, MALLOC_CAP_SPIRAM);
	if (!pValues)
	{
		message_show(SAVE_CSV_WINDOW_WIDTH, FONTID_6X8M, "������ ��� ����������", "������������ ���!", RED, 1, 1000);
		return -1;
	}

	GetThermoData(pValues);

    maxFileIndex++;

	char message[32];
	GetStringF(message, "���������� � ���� %05d%s", maxFileIndex, fileExtension);
	progress_start_show(SAVE_CSV_WINDOW_WIDTH, FONTID_6X8M, "���������� �����������", message, GREEN, 0, 0);

	char fileName[18];
	GetStringF(fileName, "/sdcard/%05d%s", maxFileIndex, fileExtension);

	FILE* f = fopen(fileName, "w");
	if (f == NULL)
	{
		message_show(SAVE_CSV_WINDOW_WIDTH, FONTID_6X8M, "������ ��� ����������", "������ ������ ����� �� ����� SD!", RED, 1, 1000);
		heap_caps_free(pValues);
		return -1;
	}

	for (uint8_t step = 0; step < 24; step++)
	{
		WriteCsvRow(f, termWidth, &pValues[step * termWidth]);
		progress_show(SAVE_CSV_WINDOW_WIDTH, FONTID_6X8M, "���������� �����������", message, GREEN, step + 1, 24);
	}

	GetStringF(message, "���� %05d%s ������� ��������", maxFileIndex, fileExtension);
	message_show(SAVE_CSV_WINDOW_WIDTH, FONTID_6X8M, "���������� �����������", message, GREEN, 0, 1000);

	fclose(f);
	fflush(f);

	heap_caps_free(pValues);

	return 0;
}
//==============================================================================

//==============================================================================
// ������� ���������� �������� ����� ������� �� ����� SD
// (16 ������ ������ BMP, ���������� �������� ��� ������ 15)
//==============================================================================
int save_ImageBMP_15bit(void)
{
	uint16_t screenWidth = dispcolor_getWidth();
	uint16_t screenHeight = dispcolor_getHeight();
	char fileExtension[] = ".BMP";

	// ������������� ����� �� ������ ���� ��� �� ���� ���������������� ��� ���������� ����� �������������
	sd_CardInit();

	int32_t maxFileIndex = GetLastIndex(fileExtension);
	if (maxFileIndex < 0)	// �� ������� ������� �����
    {
		message_show(SAVE_BMP_WINDOW_WIDTH, FONTID_6X8M, "������ ��� ����������", "������ ������� � ����� SD", RED, 1, 1000);
	    return -1;
    }

	// �������� ��������� ����� � ������ ��� �������� �������� �����
	uint16_t *pScreen = heap_caps_malloc((screenWidth * screenHeight) << 1, MALLOC_CAP_SPIRAM);//MALLOC_CAP_INTERNAL);
	if (!pScreen)
	{
		message_show(SAVE_BMP_WINDOW_WIDTH, FONTID_6X8M, "������ ��� ����������", "������������ ���!", RED, 1, 1000);
		return -1;
	}

	dispcolor_getScreenData(pScreen);

    maxFileIndex++;

	char message[32];
	GetStringF(message, "���������� � ���� %05d%s", maxFileIndex, fileExtension);
	progress_start_show(SAVE_BMP_WINDOW_WIDTH, FONTID_6X8M, "���������� �����������", message, GREEN, 0, 0);

	char fileName[18];
	GetStringF(fileName, "/sdcard/%05d%s", maxFileIndex, fileExtension);

	FILE* f = fopen(fileName, "w");
	if (f == NULL)
	{
		message_show(SAVE_BMP_WINDOW_WIDTH, FONTID_6X8M, "������ ��� ����������", "������ ������ ����� �� ����� SD!", RED, 1, 1000);
		heap_caps_free(pScreen);
		return -1;
	}

	WriteBmpFileHeader3(f, 16, screenWidth, screenHeight);

	for (uint8_t step = 0; step < 24; step++)
	{
		for (int row = 0; row < 10; row++)
			WriteBmpRow_15bit(f, screenWidth, &pScreen[(screenHeight - (step * 10 + row) - 1) * screenWidth]);

		progress_show(SAVE_BMP_WINDOW_WIDTH, FONTID_6X8M, "���������� �����������", message, GREEN, step + 1, 24);
	}

	GetStringF(message, "���� %05d%s ������� ��������", maxFileIndex, fileExtension);
	message_show(SAVE_BMP_WINDOW_WIDTH, FONTID_6X8M, "���������� �����������", message, GREEN, 0, 1000);

	fclose(f);
	fflush(f);

	heap_caps_free(pScreen);

	return 0;
}
//==============================================================================

//==============================================================================
// ������� ���������� �������� ����� ������� �� ����� SD
// (24 (32) ������ ������ BMP, ���������� �������� ��� ������ 16)
//==============================================================================
int save_ImageBMP_24bit(void)
{
	uint16_t screenWidth = dispcolor_getWidth();
	uint16_t screenHeight = dispcolor_getHeight();
	char fileExtension[] = ".BMP";

	// ������������� ����� �� ������ ���� ��� �� ���� ���������������� ��� ���������� ����� �������������
	sd_CardInit();

	int32_t maxFileIndex = GetLastIndex(fileExtension);
	if (maxFileIndex < 0)	// �� ������� ������� �����
    {
		message_show(SAVE_BMP_WINDOW_WIDTH, FONTID_6X8M, "������ ��� ����������", "������ ������� � ����� SD", RED, 1, 1000);
	    return -1;
    }

	// �������� ��������� ����� � ������ ��� �������� �������� �����
	uint16_t *pScreen = heap_caps_malloc((screenWidth * screenHeight) << 1, MALLOC_CAP_SPIRAM);
	if (!pScreen)
	{
		message_show(SAVE_BMP_WINDOW_WIDTH, FONTID_6X8M, "������ ��� ����������", "������������ ���!", RED, 1, 1000);
		return -1;
	}

	dispcolor_getScreenData(pScreen);

    maxFileIndex++;

	char message[32];
	GetStringF(message, "���������� � ���� %05d%s", maxFileIndex, fileExtension);
	progress_start_show(SAVE_BMP_WINDOW_WIDTH, FONTID_6X8M, "���������� �����������", message, GREEN, 0, 0);

	char fileName[18];
	GetStringF(fileName, "/sdcard/%05d%s", maxFileIndex, fileExtension);

	FILE* f = fopen(fileName, "w");
	if (f == NULL)
	{
		message_show(SAVE_BMP_WINDOW_WIDTH, FONTID_6X8M, "������ ��� ����������", "������ ������ ����� �� ����� SD!", RED, 1, 1000);
		heap_caps_free(pScreen);
		return -1;
	}

	WriteBmpFileHeaderCore(f, 24, screenWidth, screenHeight);

	for (uint8_t step = 0; step < 24; step++)
	{
		for (int row = 0; row < 10; row++)
			WriteBmpRow_24bit(f, screenWidth, &pScreen[(screenHeight - (step * 10 + row) - 1) * screenWidth]);

		progress_show(SAVE_BMP_WINDOW_WIDTH, FONTID_6X8M, "���������� �����������", message, GREEN, step + 1, 24);
	}

	GetStringF(message, "���� %05d%s ������� ��������", maxFileIndex, fileExtension);
	message_show(SAVE_BMP_WINDOW_WIDTH, FONTID_6X8M, "���������� �����������", message, GREEN, 0, 1000);

	fclose(f);
	fflush(f);

	heap_caps_free(pScreen);

	return 0;
}
//==============================================================================

