#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "task_buttons.h"


#define BUTTON_UP_PIN		39
#define BUTTON_CENTER_PIN	37
#define BUTTON_DOWN_PIN		38

#define BUTTON_Up_Mask		(1 << 0)
#define BUTTON_Center_Mask	(1 << 1)
#define BUTTON_Down_Mask	(1 << 2)

#define LONG_PRESS_STAGE1	8
#define LONG_PRESS_STAGE2	50
#define LONG_PRESS_STAGE2_ACCEL		4	// Коэффициент, во сколько раз удержание кнопки в STAGE2 быстрее, чем в STAGE1

static uint8_t ButtonsStateOld = 0x7;
QueueHandle_t xButtonsEventQueue;


void buttons_init()
{
    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = BIT64(BUTTON_UP_PIN) | BIT64(BUTTON_CENTER_PIN) | BIT64(BUTTON_DOWN_PIN);
    io_conf.pull_down_en = false;
    io_conf.pull_up_en = false;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
}

void buttons_task(void* arg)
{
	xButtonsEventQueue = xQueueCreate(20, sizeof(uint8_t));
    if (xButtonsEventQueue == NULL)
    {
		vTaskDelete(NULL);
		return;
    }
/*
	vTaskDelay(2000 / portTICK_RATE_MS);
    while (1)
    {
    	uint8_t event0 = ShortPress_Up;
		xQueueSend(xButtonsEventQueue, (void *) &event0, (TickType_t) 10);
		vTaskDelay(3000 / portTICK_RATE_MS);
    	event0 = ShortPress_Down;
		xQueueSend(xButtonsEventQueue, (void *) &event0, (TickType_t) 10);
		vTaskDelay(8000 / portTICK_RATE_MS);
    }*/
	uint32_t counterUp = 0, counterDown = 0;
	uint8_t stageCounterUp = 0, stageCounterDown = 0;

    while (1)
    {
    	uint8_t ButtonsState = 0;
    	ButtonsState |= gpio_get_level(BUTTON_UP_PIN) ? 0 : BUTTON_Up_Mask;
    	ButtonsState |= gpio_get_level(BUTTON_CENTER_PIN) ? 0 : BUTTON_Center_Mask;
    	ButtonsState |= gpio_get_level(BUTTON_DOWN_PIN) ? 0 : BUTTON_Down_Mask;

		// Кнопка UP в нажатом состоянии
    	if (ButtonsState & BUTTON_Up_Mask)
    	{
			uint8_t event = ShortPress_Up;

			if (ButtonsStateOld & BUTTON_Up_Mask)	// Кнопка удерживается
    		{
    			counterUp++;
    			if (counterUp > LONG_PRESS_STAGE2)
    			{
        			xQueueSend(xButtonsEventQueue, (void *) &event, (TickType_t) 10);
    			}
    			else if (counterUp > LONG_PRESS_STAGE1)
    			{
    				if (++stageCounterUp == LONG_PRESS_STAGE2_ACCEL)
    				{
    					stageCounterUp = 0;
            			xQueueSend(xButtonsEventQueue, (void *) &event, (TickType_t) 10);
    				}
    			}
    		}
    		else									// Кнопка только что нажата
    		{
    			counterUp = stageCounterUp = 0;
    			xQueueSend(xButtonsEventQueue, (void *) &event, (TickType_t) 10);
    		}
    	}

    	// Кнопка CENTER только что нажата
    	if (!(ButtonsStateOld & BUTTON_Center_Mask) && (ButtonsState & BUTTON_Center_Mask))
    	{
    		uint8_t event = ShortPress_Center;
    		xQueueSend(xButtonsEventQueue, (void *) &event, (TickType_t) 10);
    	}

		// Кнопка DOWN в нажатом состоянии
    	if (ButtonsState & BUTTON_Down_Mask)
    	{
    		uint8_t event = ShortPress_Down;

			if (ButtonsStateOld & BUTTON_Down_Mask)	// Кнопка удерживается
    		{
    			counterDown++;
    			if (counterDown > LONG_PRESS_STAGE2)
    			{
    				xQueueSend(xButtonsEventQueue, (void *) &event, (TickType_t) 10);
    			}
    			else if (counterDown > LONG_PRESS_STAGE1)
    			{
    				if (++stageCounterDown == LONG_PRESS_STAGE2_ACCEL)
    				{
    					stageCounterDown = 0;
            			xQueueSend(xButtonsEventQueue, (void *) &event, (TickType_t) 10);
    				}
    			}
    		}
    		else									// Кнопка только что нажата
    		{
    			counterDown = stageCounterDown = 0;
    			xQueueSend(xButtonsEventQueue, (void *) &event, (TickType_t) 10);
    		}
    	}

    	ButtonsStateOld = ButtonsState;

    	vTaskDelay(100 / portTICK_RATE_MS);
    }
}


