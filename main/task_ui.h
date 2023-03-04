#ifndef MAIN_TASK_UI_H_
#define MAIN_TASK_UI_H_



extern uint8_t Need2RedrawTitle;
extern uint8_t Need2RedrawPalette;
extern float minTempNew;
extern float maxTempNew;


int8_t AllocThermoImageBuffers(void);
int8_t AllocPaletteImageBuffer(void);
void ui_task(void* arg);

#endif /* MAIN_TASK_UI_H_ */
