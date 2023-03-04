#ifndef MAIN_SAVE_SAVE_H_
#define MAIN_SAVE_SAVE_H_



// Функция сохранения текущей термограммы на карту SD
// (формат CSV, разделитель значений - запятая)
int save_ImageCSV(void);
// Функция сохранения текущего кадра дисплея на карту SD
// (16 битный формат BMP, фактически значащих бит только 15)
int save_ImageBMP_15bit(void);
// Функция сохранения текущего кадра дисплея на карту SD
// (24 (32) битный формат BMP, фактически значащих бит только 16)
int save_ImageBMP_24bit(void);

#endif /* MAIN_SAVE_SAVE_H_ */
