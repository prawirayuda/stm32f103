/*
 * hw_per_reset.c
 *
 *  Created on: Jun 5, 2023
 *      Author: muham
 */

#include "stm32f1xx.h"
#include "cmsis_os.h"
#include "hw_per_reset.h"
#include "state_machine.h"
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define TEST_UART huart3


osThreadId_t hwPerTaskHandle;
const osThreadAttr_t hwPerTask_attributes = {
	.name = "hwPerTask",
	.stack_size = 128*4,
	.priority = (osPriority_t) osPriorityNormal
};

osEventFlagsId_t hw_per_event_flag;


state_handler periodical_hw_reset_handlers_tbl[] = {
    handler_ton,
	handler_toff,
};

struct tsm_state_machine_hw_reset_s per_hw_reset_sm = {
    .state = PERIODICAL_HW_RESET_STATE_ON_E,
    .dispatcher = per_hw_timer_dispatcher,
//    .event_flag = 0x01,
    .handlers = periodical_hw_reset_handlers_tbl
};

void per_hw_timer_dispatcher(void){
    (per_hw_reset_sm.handlers)[per_hw_reset_sm.state]();
    osEventFlagsWait(hw_per_event_flag, EVENT_FLAG_HW_RESET_E, osFlagsWaitAny, osWaitForever);
}

void handler_ton(void){
	//change to right hardreset pin
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin,GPIO_PIN_RESET);
	//for testing
//	uart_buf_len = sprintf(uart_buf, "TURN OFF\r\n");
//	HAL_UART_Transmit(&TEST_UART, (uint8_t *) uart_buf, uart_buf_len, 100);
	//
    per_hw_reset_sm.state = PERIODICAL_HW_RESET_STATE_OFF_E;

    date_time_t timestamp;
	timestamp.year = sDate.Year;
	timestamp.month = sDate.Month;
	timestamp.day = sDate.Date;
	timestamp.hour = sTime.Hours;
	timestamp.minute = sTime.Minutes;
	timestamp.second = sTime.Seconds;

	uint32_t current_time = date_time_to_epoch(&timestamp);
	current_time = current_time + TIME_TURN_ON;
	epoch_to_date_time(&timestamp, current_time);
	uint8_t next_alarm_hour = timestamp.hour;
	uint8_t next_alarm_minute =timestamp.minute;
	uint8_t next_alarm_second = timestamp.second;
	uint8_t next_alarm_day = timestamp.day;
	set_alarm(next_alarm_day, next_alarm_hour, next_alarm_minute, next_alarm_second);

}

void handler_toff(void){
//	change to right hardreset pin
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin,GPIO_PIN_SET);
	//for testing
//	uart_buf_len = sprintf(uart_buf, "TURN ON\r\n");
//	HAL_UART_Transmit(&TEST_UART, (uint8_t *) uart_buf, uart_buf_len, 100);
	//
    per_hw_reset_sm.state = PERIODICAL_HW_RESET_STATE_ON_E;

    date_time_t timestamp;
	timestamp.year = sDate.Year;
	timestamp.month = sDate.Month;
	timestamp.day = sDate.Date;
	timestamp.hour = sTime.Hours;
	timestamp.minute = sTime.Minutes;
	timestamp.second = sTime.Seconds;

	uint32_t current_time = date_time_to_epoch(&timestamp);
	current_time = current_time + TIME_TURN_OFF;
	epoch_to_date_time(&timestamp, current_time);
	uint8_t next_alarm_hour = timestamp.hour;
	uint8_t next_alarm_minute =timestamp.minute;
	uint8_t next_alarm_second = timestamp.second;
	uint8_t next_alarm_day = timestamp.day;
	set_alarm(next_alarm_day, next_alarm_hour, next_alarm_minute, next_alarm_second);

}


void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	//hardreset_alarm A
  	osEventFlagsSet(hw_per_event_flag ,EVENT_FLAG_HW_RESET_E);
}

void set_alarm(uint8_t day, uint8_t hour, uint8_t minute, uint8_t second){
//	  test_uart((uint8_t *)"set alarm");
	  sAlarm.AlarmTime.Hours = hour;
	  sAlarm.AlarmTime.Minutes = minute;
	  sAlarm.AlarmTime.Seconds = second;
	  sDate.Date = day;
	  sAlarm.Alarm = RTC_ALARM_A;
	  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

void hw_per_reset_init(void *args){
	hwPerTaskHandle = osThreadNew(hw_per_reset, NULL, &hwPerTask_attributes);
    hw_per_event_flag = osEventFlagsNew(NULL);

}

void hw_per_reset(void *args){
	while(1){
		HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN); // call alarm_A
		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN); //it a must
		HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN); //it a must
		//for testing only
//		date_buffer_len = sprintf(date,"Date: %02d.%02d.%02d\t",sDate.Date,sDate.Month,sDate.Year);
//		HAL_UART_Transmit(&TEST_UART, (uint8_t *) date	, date_buffer_len, 100);
//		time_buffer_len = sprintf(time,"Time: %02d.%02d.%02d\r\n",sTime.Hours,sTime.Minutes,sTime.Seconds);
//		HAL_UART_Transmit(&TEST_UART, (uint8_t *) time	, time_buffer_len, 100);
//		osDelay(1000);
		//for testing only
		per_hw_reset_sm.dispatcher();
	}

	// sprintf(date,"Date: %02d.%02d.%02d\t",sDate.Date,sDate.Month,sDate.Year);
	// sprintf(time,"Time: %02d.%02d.%02d\r\n",sTime.Hours,sTime.Minutes,sTime.Seconds);
	// HAL_UART_Transmit(&huart1, (uint8_t *)date, sizeof(date), 100);
	// HAL_UART_Transmit(&huart1, (uint8_t *)time, sizeof(time), 100);
}


void TSM_RTC_IRQ_Handler(RTC_HandleTypeDef *hrtc){

  if (__HAL_RTC_ALARM_GET_IT_SOURCE(hrtc, RTC_IT_ALRA))
  {
	/* Get the status of the Interrupt */
	if (__HAL_RTC_ALARM_GET_FLAG(hrtc, RTC_FLAG_ALRAF) != (uint32_t)RESET)
	{
	  /* AlarmA callback */
#if (USE_HAL_RTC_REGISTER_CALLBACKS == 1)
	  hrtc->AlarmAEventCallback(hrtc);
#else
	  HAL_RTC_AlarmAEventCallback(hrtc);
#endif /* USE_HAL_RTC_REGISTER_CALLBACKS */

	  /* Clear the Alarm interrupt pending bit */
	  __HAL_RTC_ALARM_CLEAR_FLAG(hrtc, RTC_FLAG_ALRAF);
	}
  }

  /* Clear the EXTI's line Flag for RTC Alarm */
  __HAL_RTC_ALARM_EXTI_CLEAR_FLAG();

  /* Change RTC state */
  hrtc->State = HAL_RTC_STATE_READY;
}
