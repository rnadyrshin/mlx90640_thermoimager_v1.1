#ifndef MAIN_ADC_ADC_H_
#define MAIN_ADC_ADC_H_

#include <stdio.h>
#include <string.h>
#include "esp_system.h"



uint32_t getBatteryVoltage();
esp_err_t init_adc();


#endif /* MAIN_ADC_ADC_H_ */
