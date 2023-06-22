/*
 * wdt_task.c
 *
 *  Created on: Jun 21, 2023
 *      Author: muham
 */

#ifndef SRC_WDT_TASK_C_
#define SRC_WDT_TASK_C_

#include "wdt_task.h"
#include "stm32f1xx_hal.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>

extern osEventFlagsId_t atc_xfer_event_group;
extern urc_expected_data_s response_wdt_command;
extern TIM_HandleTypeDef htim3;
extern void Error_Handler(void);
extern RTC_TimeTypeDef sTime;
extern RTC_HandleTypeDef hrtc;
extern UART_HandleTypeDef huart3;

osEventFlagsId_t wdtEventFlagHandle;
osEventFlagsId_t wdt_response_event_flag_handle;

osThreadId_t wdtTaskHandle;
const osThreadAttr_t wdtTask_attributes = {
  .name = "wdtTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};



void delay_us(uint16_t us){
	__HAL_TIM_SET_COUNTER(&htim3, us-1);
	__HAL_TIM_ENABLE(&htim3);
//	while(__HAL_TIM_GET_COUNTER(&htim3) < us);
}

void modem_at_handle(uint8_t *data, uint16_t len){
//	osStatus_t ret;
//	uint8_t *resp_error = data;
//	uint16_t data_urc_len = len;
//
//	if ((response_wdt_command.urc_error_check == 0)&&(strncmp((char *)resp_error, "ERROR", data_urc_len)!=0)){
//		osEventFlagsSet(atc_xfer_event_group, ATC_XFER_COMPLETED_E);
//	}
	return;
}


struct internal_atc_xfer_s wdt_command_check[] = {
	{
		.packet_data = {"AT"},
		.packet_len = 2,
		.timeout_ms = 3000,
		.atc_mode = ATC_TYPE_WRITE_E,
		.cmd_number = 0
	}
};

struct atc_xfer_req_s wdt_response[] = {
	{
			.keyword = {"OK"},
			.keyword_len = 2,
			.cb_fn = modem_at_handle,
	}
};




void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  if (htim->Instance == TIM3){
	  __HAL_TIM_DISABLE(&htim3);
	  __HAL_TIM_CLEAR_FLAG(&htim3, TIM_IT_UPDATE);
	  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
//	  osEventFlagsSet(wdtEventFlagHandle, EVENT_FLAG_WDT_E);
  }
  /* USER CODE END Callback 1 */
}

void wdt_task_init(void *args){
	wdt_response_event_flag_handle = osEventFlagsNew(NULL);
	wdtEventFlagHandle = osEventFlagsNew(NULL);
	osEventFlagsSet(wdtEventFlagHandle, EVENT_FLAG_WDT_E);
//	urc_data.connectivity_check = 1;
	wdtTaskHandle = osThreadNew(wdt_task, NULL, &wdtTask_attributes);
}

static uint8_t RTC_ByteToBcd(uint8_t Value)
{
  uint32_t bcdhigh = 0U;

  while (Value >= 10U)
  {
    bcdhigh++;
    Value -= 10U;
  }

  return ((uint8_t)(bcdhigh << 4U) | Value);
}

void wdt_task(){

//	if(HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
//	{
//		Error_Handler();
//	}

//	HAL_TIM_Base_Start(&htim3);
//	timer_val = __HAL_TIM_GET_COUNTER(&htim3);
//	delay_us(300);
//	timestamp = HAL_RTC_GetTime(&hrtc, &sTime,FORMAT_BIN);
	uint8_t timer;
	while(1){
		osDelay(800);

		HAL_RTC_GetTime(&hrtc, &sTime,FORMAT_BIN);
		timer = (uint8_t)RTC_ByteToBcd(sTime.Seconds);
		if(timer == 0x00){
			struct internal_atc_xfer_s *curr_process = &wdt_command_check[0];
			xfer_at_command(curr_process);
			osThreadYield();
			//check is response OK
			if(){
				//reset modem
				HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
			}else{
				HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
			}
		}
//			uint8_t test[] = "AT";
//			HAL_UART_Transmit(&huart3, test, sizeof(test), 100);

//		uint8_t time_buffer_len = sprintf(timestamp,"%02d\r\n",sTime.Seconds);
//		if(__HAL_TIM_GET_COUNTER(&htim3) - timer_val >= 1000)
//		{
//			HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
//			timer_val = __HAL_TIM_GET_COUNTER(&htim3);
//
//		}

//		osThreadYield();
//		osDelay(1000);
//		osEventFlagsClear(boot_sequence_event_group, BOOT_SEQ_REQUESTED_E);
//		osEventFlagsSet(boot_sequence_event_group, BOOT_SEQ_ATC_CALL_COMPLETED_E);


	}
}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//  /* Prevent unused argument(s) compilation warning */
//  UNUSED(htim);
//
//  /* NOTE : This function should not be modified, when the callback is needed,
//            the HAL_TIM_PeriodElapsedCallback could be implemented in the user file
//   */
//}
#endif /* SRC_WDT_TASK_C_ */
