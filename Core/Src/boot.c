/*
 * boot.c
 *
 *  Created on: Jun 20, 2023
 *      Author: muham
 */



#include "boot.h"

int handle_rat_setup_cb(uint8_t *, uint16_t);
void complete_boot(void);

osEventFlagsId_t boot_sequence_event_group;
//osEventFlagsId_t boot_sequence_event_group;
extern osEventFlagsId_t atc_xfer_event_group;
extern urc_expected_data_s urc_data;
extern osEventFlagsId_t atc_xfer_ready;

uint32_t wait2_flag;


//uint32_t ready_xfer_flag = ATC_XFER_READY_E;

struct internal_atc_xfer_s boot_sequence[] = {
		{NULL}
};

struct internal_atc_xfer_s data_call[] = {
		{
				.packet_data = {"AT+CFUN=1,1"},
				.packet_len = 11,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 0,
		},
		{
				.packet_data = {"AT"},
				.packet_len = 2,
				.cb_fn = handle_rat_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 0
		},
		{
				.packet_data = {"AT+CPIN?"},
				.packet_len = 8,
				.cb_fn = handle_rat_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_READ_E,
				.cmd_number = 1
		},
		{
				.packet_data = {"AT+CGREG?"},
				.packet_len = 9,
				.cb_fn = handle_rat_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_READ_E,
				.cmd_number = 2
		},
		{
				.packet_data = {"AT+COPS?"},
				.packet_len = 8,
				.cb_fn = handle_rat_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_READ_E,
				.cmd_number = 3
		},
		{
				.packet_data = {"AT+COPS=1,2,\"51010\",7"},
				.packet_len = 21,
				.cb_fn = handle_rat_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 4
		},
//		{
//				.packet_data = {"AT+CGDCONT=1,\"IP\",\"M2MAUTOTRONIC\","},
//				.packet_len = 34,
//				.cb_fn = handle_rat_setup_cb,
//				.timeout_ms = 500,
//				.atc_mode = ATC_TYPE_WRITE_E,
//				.cmd_number = 5
//		},
//		{
//				.packet_data = {"AT+CGATT=1"},
//				.packet_len = 10,
//				.cb_fn = handle_rat_setup_cb,
//				.timeout_ms = 500,
//				.atc_mode = ATC_TYPE_WRITE_E,
//				.cmd_number = 6
//		},
//		{
//				.packet_data = {"AT+CGACT=1,1"},
//				.packet_len = 12,
//				.cb_fn = handle_rat_setup_cb,
//				.timeout_ms = 500,
//				.atc_mode = ATC_TYPE_WRITE_E,
//				.cmd_number = 7
//		},
//		{
//				.packet_data = {"AT+CGDCONT?"},
//				.packet_len = 11,
//				.cb_fn = handle_rat_setup_cb,
//				.timeout_ms = 500,
//				.atc_mode = ATC_TYPE_WRITE_E,
////				.atc_mode = ATC_TYPE_WRITE_E,
//				.cmd_number = 8
////				.cmd_number = 0
//		},
//		{
//				.packet_data = {"AT+CGATT=0"},
//				.packet_len = 10,
//				.cb_fn = handle_rat_setup_cb,
//				.timeout_ms = 500,
//				.atc_mode = ATC_TYPE_WRITE_E,
//				.cmd_number = 9
//		},
		{NULL}
};

osThreadId_t bootTaskHandle;
const osThreadAttr_t bootTask_attributes = {
  .name = "bootTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};


int handle_rat_setup_cb(uint8_t *data, uint16_t len){
	return 0;
}

void boot_task(void *args){
//	osEventFlagsWait(boot_sequence_event_group, BOOT_SEQ_REQUESTED_E, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

	uint32_t received_flag;
	uint32_t wait_flag;
	int start_atc;
	while (1){
//
		wait_flag = osEventFlagsWait(boot_sequence_event_group, BOOT_SEQ_REQUESTED_E, osFlagsWaitAny, osWaitForever);
//		if (wait_flag & BOOT_EVENT_REQUESTED_E) {

		struct internal_atc_xfer_s *curr_process = &data_call[0];
		start_atc = urc_data.connectivity_check;
		// iterate the boot sequence table
		for (curr_process = &data_call[start_atc]; curr_process->packet_len != 0; curr_process++){
				xfer_at_command(curr_process);
				received_flag = osEventFlagsWait(atc_xfer_event_group, ATC_XFER_COMPLETED_E | ATC_XFER_TIMEOUT_E, osFlagsWaitAny, osWaitForever);

				if (received_flag & ATC_XFER_COMPLETED_E){
				// Do something
					goto next;
				}else if (received_flag & ATC_XFER_TIMEOUT_E){
					// TODO handle something
		//			return;
					curr_process--;
					goto next;
				}

		next:
				osThreadYield();
			}
//		}

		osEventFlagsClear(boot_sequence_event_group, BOOT_SEQ_REQUESTED_E);
		osEventFlagsSet(boot_sequence_event_group, BOOT_SEQ_ATC_CALL_COMPLETED_E);

	}
}

void wait_boot(void){
	wait2_flag = osEventFlagsWait(boot_sequence_event_group, BOOT_SEQ_ATC_CALL_COMPLETED_E, osFlagsWaitAny | osFlagsNoClear, osWaitForever);
//	return wait2_flag;
}

void boot_init(void *args){
	boot_sequence_event_group = osEventFlagsNew(NULL);
	osEventFlagsSet(boot_sequence_event_group, BOOT_SEQ_REQUESTED_E);
	urc_data.connectivity_check = 1;
	bootTaskHandle = osThreadNew(boot_task, NULL, &bootTask_attributes);
}
