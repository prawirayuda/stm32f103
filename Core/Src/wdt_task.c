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

extern osEventFlagsId_t atc_xfer_event_group;
extern urc_expected_data_s response_wdt_command;


osEventFlagsId_t wdtEventFlagHandle;

osThreadId_t wdtTaskHandle;
const osThreadAttr_t wdtTask_attributes = {
  .name = "wdtTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};





void modem_at_handle(uint8_t *data, uint16_t len){
//	osStatus_t ret;
	uint8_t *resp_error = data;
	uint16_t data_urc_len = len;

	if ((response_wdt_command.urc_error_check == 0)&&(strncmp((char *)resp_error, "ERROR", data_urc_len)!=0)){
		osEventFlagsSet(atc_xfer_event_group, ATC_XFER_COMPLETED_E);
	}
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




void wdt_task_init(void *args){
	wdtEventFlagHandle = osEventFlagsNew(NULL);
	osEventFlagsSet(wdtEventFlagHandle, EVENT_FLAG_WDT_E);
//	urc_data.connectivity_check = 1;
	wdtTaskHandle = osThreadNew(wdt_task, NULL, &wdtTask_attributes);
}


void wdt_task(){
	while(1){
		struct internal_atc_xfer_s *curr_process = &wdt_command_check[0];
//		xfer_at_command(curr_process);
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		osDelay(1000);
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
