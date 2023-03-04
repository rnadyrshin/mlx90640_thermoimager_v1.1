#ifndef MAIN_TASK_BUTTONS_H_
#define MAIN_TASK_BUTTONS_H_


#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"


#define No_Event			0
#define ShortPress_Up		1
#define ShortPress_Center	2
#define ShortPress_Down		3


extern QueueHandle_t xButtonsEventQueue;

void buttons_init();
void buttons_task(void* arg);


#endif /* MAIN_TASK_BUTTONS_H_ */
