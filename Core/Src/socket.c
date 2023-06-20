/*
 * socket.c
 *
 *  Created on: Jun 20, 2023
 *      Author: muham
 */


#include "socket.h"
#include "serial_communication.h"
#include "atc_processor.h"
#include "boot.h"

int handle_atc_setup_cb(uint8_t *, uint16_t);

extern osEventFlagsId_t atc_xfer_event_group;
volatile extern atc_processor_runtime_fsm_t atc_fsm;
//extern osMutexId_t atc_xfer_mutex;
extern osEventFlagsId_t boot_sequence_event_group;

osEventFlagsId_t socket_even_group;
osEventFlagsId_t connection_flag_event;
powerLost_flag_e pwr_lost;

uint8_t test_val;


osThreadId_t socketTaskHandle;
const osThreadAttr_t socketTask_attributes = {
		.name = "socketTask",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityNormal
};


struct socket_runtime_fsm_s {
	// protocol TCP/UDP
	uint8_t 	protocol;
	// mode server/client
	uint8_t		mode;
	// state idle/disconnected/connected/busy
	uint8_t		state;
};

// State => Handler => exe
struct internal_atc_xfer_s socket_atc_tcp_client[] = {
		{
				.packet_data = {"AT+CIPMUX=0"},
				.packet_len = 11,
				.cb_fn = handle_atc_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 16,
		},
		{
				.packet_data = {"AT+CSTT=\"M2MAUTOTRONIC\",\"\",\"\""},
				.packet_len = 35,
				.cb_fn = handle_atc_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 11,
		},
		{
				.packet_data = {"AT+CIICR"},
				.packet_len = 7,
				.cb_fn = handle_atc_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 12,
		},
		{
				.packet_data = {"AT+CIFSR"},
				.packet_len = 8,
				.cb_fn = handle_atc_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 13,
		},
		{
				.packet_data = {"AT+CIPSTART=\"TCP\",\"10.9.201.79\",3000"},
				.packet_len = 40,
				.cb_fn = handle_atc_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 15,
		},
		{
				.packet_data = {"AT+CIPSEND"},
				.packet_len = 10,
				.cb_fn = handle_atc_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 16,
		},
		{NULL}
};


struct internal_atc_xfer_s socket_atc_tcp_server[] = {
		{
				.packet_data = {"AT+CIPMUX=1"},
				.packet_len = 11,
				.cb_fn = handle_atc_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 10,
		},
		{
				.packet_data = {"AT+CSTT=\"M2MAUTOTRONIC\",\"\",\"\""},
				.packet_len = 35,
				.cb_fn = handle_atc_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 11,
		},
		{
				.packet_data = {"AT+CIICR"},
				.packet_len = 7,
				.cb_fn = handle_atc_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 12,
		},
		{
				.packet_data = {"AT+CIFSR"},
				.packet_len = 8,
				.cb_fn = handle_atc_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 13,
		},
		{
				.packet_data = {"AT+CIPSERVER=1,3000"},
				.packet_len = 19,
				.cb_fn = handle_atc_setup_cb,
				.timeout_ms = 500,
				.atc_mode = ATC_TYPE_WRITE_E,
				.cmd_number = 14,
		},
		{NULL}
};


struct socket_runtime_fsm_s socket_runtime_fsm = {
		.protocol = SOCKET_PROTOCOL_TCP_E,
		.mode = SOCKET_MODE_SERVER_E,
		.state = SOCKET_STATE_IDLE_E
};

int handle_atc_setup_cb(uint8_t *data, uint16_t len){
	return 0;
}

static void start_tcp_server(uint8_t context, uint16_t port){
//	osStatus_t  status;
	uint32_t socket_flag;
	uint32_t connection_flag;
	struct internal_atc_xfer_s *curr_process = &socket_atc_tcp_server[0];

//	osEventFlagsWait(socket_even_group, POWER_LOST_FLAG_TRUE, osFlagsWaitAny, osWaitForever);

	for (curr_process = &socket_atc_tcp_server[0]; curr_process->packet_len != 0; curr_process++){
		socket_runtime_fsm.state = SOCKET_STATE_BOOT_E;
		xfer_at_command(curr_process);
		socket_flag = osEventFlagsWait(atc_xfer_event_group, ATC_XFER_COMPLETED_E | ATC_XFER_TIMEOUT_E, osFlagsWaitAny, osWaitForever);

		if (socket_flag & ATC_XFER_COMPLETED_E){
			//do someting
			if (curr_process->cmd_number == 14 || curr_process->cmd_number == 15)
			{
				connection_flag = osEventFlagsWait(connection_flag_event, SOCKET_CONNECTED_E, osFlagsWaitAny, osWaitForever);
				socket_runtime_fsm.state = SOCKET_STATE_CONNECTED_E;
			}
			goto next;
		}else if (socket_flag & ATC_XFER_TIMEOUT_E){
			//handle something
			goto next;
		}
	next:
		osThreadYield();
	}

	atc_fsm.state = SERCOM_STATE_BRIDGE_E;
//	socket_runtime_fsm.state = SOCKET_STATE_CONNECTED_E;
	osDelay(50);
//	test_val = 10;
//	vTaskDelete(socketTaskHandle);
	osEventFlagsClear(boot_sequence_event_group, BOOT_SEQ_ATC_CALL_COMPLETED_E);
	osEventFlagsSet(boot_sequence_event_group, BOOT_SEQ_COMPLETED_E);
//	return;
}

static void start_tcp_client(char *ipaddress, uint16_t port){
	uint32_t socket_flag;
	uint32_t connection_flag;
	struct internal_atc_xfer_s *curr_process = &socket_atc_tcp_client[0];
	//dummy tes
//	osEventFlagsWait(socket_even_group, POWER_LOST_FLAG_TRUE, osFlagsWaitAny, osWaitForever);

	for (curr_process = &socket_atc_tcp_client[0]; curr_process->packet_len != 0; curr_process++){
		xfer_at_command(curr_process);

		socket_flag = osEventFlagsWait(atc_xfer_event_group, ATC_XFER_COMPLETED_E | ATC_XFER_TIMEOUT_E, osFlagsWaitAny, osWaitForever);

		if (socket_flag & ATC_XFER_COMPLETED_E){
			//do someting
			if (curr_process->cmd_number == 14 || curr_process->cmd_number == 15)
			{
				connection_flag = osEventFlagsWait(connection_flag_event, SOCKET_CONNECTED_E | SOCKET_NOT_CONNECTED_E, osFlagsWaitAny, osWaitForever);
				if (connection_flag & SOCKET_CONNECTED_E){
					socket_runtime_fsm.state = SOCKET_STATE_CONNECTED_E;
//					goto next;
				}
				else if (connection_flag & SOCKET_NOT_CONNECTED_E){
//					reboot();
					break;
				}
			}
			goto next;
		}else if (socket_flag & ATC_XFER_TIMEOUT_E){
			//handle something
			goto next;
		}
	next:
		osThreadYield();
	}
	if (!(connection_flag & SOCKET_NOT_CONNECTED_E)) atc_fsm.state = SERCOM_STATE_BRIDGE_E;
//	atc_fsm.state = SERCOM_STATE_BRIDGE_E;
//	socket_runtime_fsm.state = SOCKET_STATE_CONNECTED_E;
	osDelay(50);
//	vTaskDelete(socketTaskHandle);
	osEventFlagsSet(boot_sequence_event_group, BOOT_SEQ_COMPLETED_E);
	osEventFlagsClear(boot_sequence_event_group, BOOT_SEQ_ATC_CALL_COMPLETED_E);

}

//static void start_udp_client(char *ipaddress, uint16_t port){
//
//}

static void socket_task(void *args){
	uint32_t ret;
	while(1)
	{
		// Wait boot
//		ret = wait_boot();
		wait_boot();

		// load config
//		if (ret & BOOT_SEQ_ATC_CALL_COMPLETED_E) {
			uint16_t use_port = 3000;
			uint8_t text[] = "TEST";
			if (socket_runtime_fsm.protocol == SOCKET_PROTOCOL_TCP_E)
			{
				if (socket_runtime_fsm.mode == SOCKET_MODE_SERVER_E) start_tcp_server(*text, use_port);
				else start_tcp_client((char *)text, use_port);
			}
			else if (socket_runtime_fsm.protocol == SOCKET_PROTOCOL_UDP_E)
			{
				if (socket_runtime_fsm.mode == SOCKET_MODE_SERVER_E)
				{
					//doin something
				}
				else
				{
					//doin something
				}
			}

//		}


	//	start_tcp_server(*text, use_port);

		// setup
	}


}


void socket_init(void){
	// setup task
	socket_even_group = osEventFlagsNew(NULL);
	connection_flag_event = osEventFlagsNew(NULL);
	socketTaskHandle = osThreadNew(socket_task, NULL, &socketTask_attributes);

	// trigger FSM
	socket_runtime_fsm.protocol = SOCKET_PROTOCOL_TCP_E;
	socket_runtime_fsm.mode = SOCKET_MODE_SERVER_E;
	socket_runtime_fsm.state = SOCKET_STATE_IDLE_E;

}

int socket_get_state(void){
	return socket_runtime_fsm.state;
}

