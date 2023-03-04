#ifndef MAIN_PALETTE_PALETTE_H_
#define MAIN_PALETTE_PALETTE_H_

#include "../display/rgbcolor.h"
#include "../settings.h"


// Функция возвращает указатель на массив точек палитры выбранного типа
void getPalette(eColorScale palette, uint16_t steps, tRGBcolor *pBuff);


#endif /* MAIN_PALETTE_PALETTE_H_ */
