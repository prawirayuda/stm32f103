/*
 * wdt_task.h
 *
 *  Created on: Jun 21, 2023
 *      Author: muham
 */

#ifndef INC_WDT_TASK_H_
#define INC_WDT_TASK_H_
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#include "cmsis_os.h"
#include "atc_processor.h"


enum{
//	EVENT_FLAG_WDT_E =  0x01 << 0,
	EVENT_FLAG_WDT_E = 0x01 <<1
//	EVENT_FLAG_UART_E = 0x01 << 1
};

void wdt_task_init(void *args);
void wdt_task();

#endif /* INC_WDT_TASK_H_ */
