#ifndef MAIN_SD_SD_H_
#define MAIN_SD_SD_H_


int sd_CardInit();

int8_t sd_GetFree(uint32_t *pFreeMb, uint32_t *pTotalMb);


#endif /* MAIN_SD_SD_H_ */
