/*
 * hw_per_reset.h
 *
 *  Created on: Jun 5, 2023
 *      Author: muham
 */

#ifndef INC_HW_PER_RESET_H_
#define INC_HW_PER_RESET_H_

#include <stdio.h>
#include <stdlib.h>

#include "cmsis_os.h"
#include "stm32f1xx_hal.h"
#include "julian_date_util.h"


#define TIME_TURN_ON 3
#define TIME_TURN_OFF 5
#define HARD_RESET_CTRL_LOGIC 1



extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
//#define EXTERNAL_UART_HANDLE huart1
//
//#define USE_INTERNAL_UART 1 // using huart2
//
//#ifdef USE_INTERNAL_UART
//    extern UART_HandleTypeDef huart2;
//#endif

extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;
extern RTC_AlarmTypeDef sAlarm;
//for testing
extern char uart_buf[];
extern int uart_buf_len;
extern char time[];
extern int time_buffer_len;
extern char date[];
extern int date_buffer_len;
extern void Error_Handler(void);

enum{
    PERIODICAL_HW_RESET_STATE_ON_E,
    PERIODICAL_HW_RESET_STATE_OFF_E,
};

enum{
	EVENT_FLAG_HW_RESET_E =  0x01 << 0,
//	EVENT_FLAG_UART_E = 0x01 << 1
};



void hw_per_reset_init(void *args);
void hw_per_reset(void *args);
void set_alarm(uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
void handler_ton(void);
void handler_toff(void);
void per_hw_timer_dispatcher(void);
void TSM_RTC_IRQ_Handler(RTC_HandleTypeDef *hrtc);

#endif /* INC_HW_PER_RESET_H_ */
