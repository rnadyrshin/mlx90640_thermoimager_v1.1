#ifndef MAIN_TASK_MLX_H_
#define MAIN_TASK_MLX_H_



typedef struct
{
	float VBAT;
    float Vdd;
    float Ta;
    float ThermoImage[768];
	float CenterTemp;
	float minT;
	float maxT;
	int8_t minT_X;
	int8_t minT_Y;
	int8_t maxT_X;
	int8_t maxT_Y;
} sMlxData;

extern uint16_t *Frame;
extern sMlxData MlxData[2];
extern uint8_t MlxDataIdx;


// Процедура копирует матрицу температур в буфер pBuff
void GetThermoData(float *pBuff);

void mlx_task(void* arg);

#endif /* MAIN_TASK_MLX_H_ */
