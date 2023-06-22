/*
 * atc_processor.c
 *
 *  Created on: Jun 20, 2023
 *      Author: muham
 */



#include <stdint.h>
#include <stdbool.h>
#include "atc_processor.h"
#include "serial_communication.h"
#include "wdt_task.h"

#define ATC_TIMER_STOP osTimerStop(atc_xfer_timer)

__weak void network_tech_changed_indication(uint8_t *, uint16_t);
__weak void network_state_changed_indication(uint8_t *, uint16_t);
__weak void modem_urc_handle(uint8_t *data, uint16_t len);
__weak void error_urc_handle(uint8_t *data, uint16_t len);
//static void reboot();
static void atc_timeout_handler(void *args);

extern struct tsm_UARTHandle externalComm;
extern struct tsm_UARTHandle internalComm;
extern osEventFlagsId_t uart_xfer_event_group;
extern osEventFlagsId_t connection_flag_event;
extern osEventFlagsId_t boot_sequence_event_group;
extern osEventFlagsId_t wdt_response_event_flag_handle;

processor_handle_t urc_processor_hndl;
urc_expected_data_s urc_data;
connectivity_urc_s urc_conn;

atc_processor_runtime_fsm_t atc_fsm = {
//		.state = SERCOM_STATE_BRIDGE_E,
		.state = SERCOM_STATE_COMMAND_MODE_E,
//		.state = SERCOM_STATE_DATA_MODE_E,
		.atc_handler_cb = NULL
};

osEventFlagsId_t atc_xfer_event_group;
osMutexId_t atc_xfer_mutex;
osTimerId_t atc_xfer_timer;
const osMutexAttr_t atc_xfer_mutex_attr = {
  "atc_xfer_mutex",                          // human readable mutex name
  osMutexRecursive | osMutexPrioInherit,    // attr_bits
  NULL,                                     // memory for control block
  0U                                        // size for control block
};


__weak void network_tech_changed_indication(uint8_t *data, uint16_t len){
	osStatus_t ret;
	// Signal TX Thread data is available
	ret = osSemaphoreAcquire(urc_processor_hndl.write_semaphore, osWaitForever);
	if (ret != osOK) return; // TODO Handle something?

	// example/test response
	snprintf((char *)urc_processor_hndl.curr_write->buff, UART_MAX_FRAME_SIZE, "LED STATE: %d\n", *(data + 2) - '0');
	urc_processor_hndl.curr_write->data_len = strlen((char *)urc_processor_hndl.curr_write->buff);
	urc_processor_hndl.curr_write = urc_processor_hndl.curr_write->next;
	osSemaphoreRelease(urc_processor_hndl.read_semaphore);
//	osEventFlagsSet(uart_xfer_event_group, UART_EVENT_TX_DIR_OUT_REQUESTED_E);
//	osEventFlagsSet(atc_xfer_event_group, ATC_XFER_COMPLETED_E);
	return;
}

__weak void modem_urc_handle(uint8_t *data, uint16_t len){
	osStatus_t ret;
	uint8_t *resp_error = data;
	uint16_t data_urc_len = len;


//	err_urc = error_urc_handle();
	if ((urc_data.urc_error_check == 0)&&(strncmp((char *)resp_error, "ERROR", data_urc_len)!=0)){
		osEventFlagsSet(atc_xfer_event_group, ATC_XFER_COMPLETED_E);
	}else{
		switch(urc_processor_hndl.urc_cmp_num){
		case 1:
			//doing something
			osEventFlagsSet(atc_xfer_event_group, ATC_XFER_COMPLETED_E);
			break;
		case 2:
			//doing something
			osEventFlagsSet(atc_xfer_event_group, ATC_XFER_TIMEOUT_E);
			break;
		case 3:
			//doing smothing
			osEventFlagsSet(atc_xfer_event_group, ATC_XFER_TIMEOUT_E);
			break;
		case 4:
			//doing smothing
			osEventFlagsSet(atc_xfer_event_group, ATC_XFER_TIMEOUT_E);
			break;
		case 11:
			//doing smothing
			osEventFlagsSet(atc_xfer_event_group, ATC_XFER_TIMEOUT_E);
			break;
		case 12:
			//doing smothing
			osEventFlagsSet(atc_xfer_event_group, ATC_XFER_TIMEOUT_E);
			break;
		case 13:
			//doing smothing
			osEventFlagsSet(atc_xfer_event_group, ATC_XFER_TIMEOUT_E);
			break;
		case 14:
			//doing smothing
			osEventFlagsSet(atc_xfer_event_group, ATC_XFER_TIMEOUT_E);
			break;
		case 15:
			//doing smothing
			osEventFlagsSet(atc_xfer_event_group, ATC_XFER_TIMEOUT_E);
			break;
		case 16:
			//doing smothing
			osEventFlagsSet(atc_xfer_event_group, ATC_XFER_TIMEOUT_E);
			break;
		default:
			break;
		}
	}
	return;
}

__weak void error_urc_handle(uint8_t *data, uint16_t len){
//	osStatus_t ret;
	uint8_t *connection_data = data;
	uint16_t len_connection = len;
	//doing something
	urc_data.urc_error_check = 0;
	if (strncmp((char *)connection_data, "SERVER OK", len_connection)==0)
	{
		//doing something
		urc_conn.get_connectivity = 1;
	}
	else if (strncmp((char *)connection_data, "CONNECT OK", len_connection)==0)
	{
		//doing something
		osEventFlagsClear(connection_flag_event, SOCKET_NOT_CONNECTED_E);
		osEventFlagsSet(connection_flag_event, SOCKET_CONNECTED_E);
	}
	else if (strncmp((char *)connection_data, "CONNECT FAIL", len_connection)==0)
	{
		//doing something
//		osEventFlagsClear(connection_flag_event, SOCKET_CONNECTED_E);
//		osEventFlagsSet(connection_flag_event, SOCKET_NOT_CONNECTED_E);
		reboot();
	}
	else if (strncmp((char *)connection_data, "0,CLOSED", len_connection)==0)
	{
		//doing something
		reboot();
	}
	else if (strncmp((char *)connection_data, "\0", len_connection)!=0)
	{
		//doing something
		if (urc_conn.get_connectivity == 1)
		{
			//adding led indicator pattern for connection
			osEventFlagsSet(connection_flag_event, SOCKET_CONNECTED_E);
			urc_conn.get_connectivity = 0;
		}
	}
	return;
}

void reboot(){
	//doing reboot
	urc_data.connectivity_check = 0;
	atc_fsm.state = SERCOM_STATE_COMMAND_MODE_E;
	memset(internalComm.circularReceiver.rx_buff, 0x00, UART_BUFFER_SIZE);
	memset(externalComm.circularReceiver.rx_buff, 0x00, UART_BUFFER_SIZE);
	memset(urc_processor_hndl.curr_read->buff, 0x00, sizeof(urc_processor_hndl.curr_read->buff));
	memset(urc_processor_hndl.curr_write->buff, 0x00, sizeof(urc_processor_hndl.curr_write->buff));
//	xfer_at_command(reset_command);
	osEventFlagsClear(boot_sequence_event_group, BOOT_SEQ_ATC_CALL_COMPLETED_E);
	osEventFlagsSet(boot_sequence_event_group, BOOT_SEQ_REQUESTED_E);
	osEventFlagsSet(connection_flag_event, SOCKET_NOT_CONNECTED_E);
	return;
}


__weak void network_state_changed_indicaiton(uint8_t *data, uint16_t len){
	return;
}


struct atc_xfer_req_s global_urc_table[] = {
	{
			.keyword = {"OK"},
			.keyword_len = 2,
			.cb_fn = modem_urc_handle,
	},
	{
			.keyword = {">"},
			.keyword_len = 1,
			.cb_fn = modem_urc_handle,
	},
	{
			.keyword = {"AT READY"},
			.keyword_len = 8,
			.cb_fn = modem_urc_handle,
	},
	{
			.keyword = {"ERROR"},
			.keyword_len = 5,
			.cb_fn = modem_urc_handle,
	},
	{
			.keyword = {"+CPIN: READY"},
			.keyword_len = 12,
			.cb_fn = error_urc_handle,
			.urc_read_nmbr = 1,
	},
	{
			.keyword = {"+CGREG: 0,1"},
			.keyword_len = 11,
			.cb_fn = error_urc_handle,
			.urc_read_nmbr = 2,
	},
	{
			.keyword = {"+COPS: 1,2,\"51010\",7"},
			.keyword_len = 20,
			.cb_fn = error_urc_handle,
			.urc_read_nmbr = 3,
	},
	{
			.keyword = {"+CGACT: 1,1,10.66.41.251"},
			.keyword_len = 22,
			.cb_fn = error_urc_handle,
			.urc_read_nmbr = 7,
	},
	{
			.keyword = {"+CGDCONT: 0,\"IP\",\"M2MAUTOTRONIC\",\"10.66.41.251\""},
			.keyword_len = 45,
			.cb_fn = error_urc_handle,
			.urc_read_nmbr = 8,
	},
	{
			.keyword = {"+CGDCONT: 1,\"IP\",\"M2MAUTOTRONIC\",\"10.66.41.251\",0,0"},
			.keyword_len = 49,
			.cb_fn = error_urc_handle,
			.urc_read_nmbr = 8,
	},
	{
			.keyword = {"+SIM NOT INSERT"},
			.keyword_len = 8,
			.cb_fn = error_urc_handle,
			.urc_read_nmbr = 0,
	},
	{
			.keyword = {"0,CLOSED"},
			.keyword_len = 8,
			.cb_fn = error_urc_handle,
			.urc_read_nmbr = 0,
	},
	{
			.keyword = {"SERVER OK"},
			.keyword_len = 8,
			.cb_fn = error_urc_handle,
			.urc_read_nmbr = 0,
	},
	{
			.keyword = {"CONNECT OK"},
			.keyword_len = 8,
			.cb_fn = error_urc_handle,
			.urc_read_nmbr = 0,
	},
	{
			.keyword = {"CONNECT FAIL"},
			.keyword_len = 8,
			.cb_fn = error_urc_handle,
			.urc_read_nmbr = 0,
	},
	{NULL}
};

void data_bridge_dir_out_notify(void){
	// release the semaphore back to write semaphore
	osSemaphoreRelease(urc_processor_hndl.write_semaphore);

	// set next read pointer to curr->next
	urc_processor_hndl.curr_read = urc_processor_hndl.curr_read->next;
}

void atc_xfer_timeout_cb(void *args){
	atc_timeout_handler(NULL);
}

void atc_init(void){
	uint32_t xfer_timer_cb_arg = 0x01;
	// Initialize Semaphores
	urc_processor_hndl.write_semaphore = osSemaphoreNew(2U, 2U, NULL);
	urc_processor_hndl.read_semaphore = osSemaphoreNew(2U, 0U, NULL);

	// Initialize read and write pointers
	urc_processor_hndl.buff[0].next = &urc_processor_hndl.buff[1];
	urc_processor_hndl.buff[1].next = &urc_processor_hndl.buff[0];
	urc_processor_hndl.curr_read = &urc_processor_hndl.buff[0];
	urc_processor_hndl.curr_write = &urc_processor_hndl.buff[0];

	// initialize mutex
	// TODO Mutex create might fail?
	atc_xfer_mutex = osMutexNew(&atc_xfer_mutex_attr);
	atc_xfer_event_group = osEventFlagsNew(NULL);
	atc_xfer_timer = osTimerNew(atc_xfer_timeout_cb, osTimerOnce, &xfer_timer_cb_arg, NULL);

	urc_data.urc_error_check = 0;

}

static __inline void atc_state_reset(void){
	osStatus_t  status;
		// Get mutex
	status = osMutexAcquire(atc_xfer_mutex, osWaitForever);

	// reset state
	atc_fsm.atc_handler_cb = NULL;
	atc_fsm.state = SERCOM_STATE_IDLE_E;

	osMutexRelease(atc_xfer_mutex);
}

int atc_intermediate_dispatcher(uint8_t *data, uint16_t len){
	int status;
//	osStatus_t  status;

	//TODO:
	//STOPO OS TIMER
	status = osTimerIsRunning(atc_xfer_timer);
	if(status == 1) osTimerStop(atc_xfer_timer);
	else osTimerStop(atc_xfer_timer);

	// Call the handler
	if (!atc_fsm.atc_handler_cb) return -1;

	// if return OK, stop timer reset state
	status = atc_fsm.atc_handler_cb(data, len);

	if (0 == status){ // if OK
		atc_state_reset();
	}

	return status;
}

static void atc_timeout_handler(void *args){
	atc_state_reset();
	atc_intermediate_dispatcher(NULL, 0);
//	osEventFlagsSet(atc_xfer_event_group, ATC_XFER_TIMEOUT_E);
	// TODO do something else??
}

static void get_ip_client(uint8_t *data, uint16_t len){
	char *conn_data = (char *)data;
	char *find_comma;
	char *find_collon;
	int client_len;
	int ip_len;

	find_comma = strchr(conn_data, ',');
	client_len = (int)(find_comma - conn_data);
	find_collon = strchr(conn_data, ':');
	ip_len = (int)(find_collon - conn_data);
	strncpy(urc_conn.client_number, &conn_data[client_len - 1], 1);
	strncpy(urc_conn.ip, &conn_data[ip_len + 2], (strlen(conn_data)- (ip_len + 2)));
	uint16_t len_ip_client = (strlen(conn_data)- (ip_len + 2));


	error_urc_handle((uint8_t *)urc_conn.ip, len_ip_client);
}

static void dispatch_urc_as_newline_terminated(uint8_t *data, uint16_t len){
	uint16_t length_left;
	uint16_t curr_partial_len;
//	uint16_t data_len = len;
	uint8_t *data_head = data;
	uint8_t *next_termination = data_head;
	struct atc_xfer_req_s *command_ptr = NULL;
	bool handled = false;

	do {
		// loop until next termination
		for (length_left = len-1; *next_termination != '\r' && *next_termination != '\n' && length_left; next_termination++, length_left--);
		if (*next_termination != '\r' && *next_termination != '\n') break;
		*next_termination = '\0';
		curr_partial_len = (uint32_t)next_termination - (uint32_t)data_head;
		if (curr_partial_len == 0 && (strncmp((char *)data_head, "\0", curr_partial_len)!=0)) continue;

		// if in command_mode, is command response
		if (atc_fsm.state == SERCOM_STATE_COMMAND_MODE_E){
			atc_intermediate_dispatcher(data_head, curr_partial_len);
		}else{
			// search global table
			//check read or write mode
			if (urc_processor_hndl.urc_cmp_num != 0){
				//change " to *
				if (strncmp((char *)data_head, "OK", curr_partial_len)!=0){
					//save data to identify error
					memcpy((char *)urc_data.packet_urc, (char *)data_head, curr_partial_len);
					urc_data.urc_len = curr_partial_len;
					urc_data.urc_ok = 0;

					if(urc_conn.get_connectivity == 1)
					{
						get_ip_client((uint8_t *)urc_data.packet_urc, urc_data.urc_len);
					}
				}

			}

			command_ptr = &global_urc_table[0];
			while (*command_ptr->keyword){
				if (0 == strncmp(command_ptr->keyword, (char *)data_head, command_ptr->keyword_len)){
					urc_data.urc_ok = 1;
					break;
				}
				command_ptr++;
			}
			if (*command_ptr->keyword && command_ptr->cb_fn != NULL) command_ptr->cb_fn(data_head, curr_partial_len);
		}

		data_head = next_termination + 1;
		osThreadYield();
	}while (length_left);
}


void internal_receive_urc(void){
	osStatus_t ret;
	struct uartDataQueue rxQueue;
	ret = osMessageQueueGet(internalComm.rxQueue, &rxQueue, 0U, osWaitForever);


	// Check if is probably an URC
	bool isNewline = is_uart_queue_data_newline_terminated(internalComm.circularReceiver.rx_buff, &rxQueue);

	if (isNewline){
		// Dequeue to buff
		uart_dequeue_to_buffer((char *)urc_processor_hndl.processing_buff, &urc_processor_hndl.processing_data_size, internalComm.circularReceiver.rx_buff, &rxQueue);
		osEventFlagsSet(wdt_response_event_flag_handle, EVENT_FLAG_WDT_RESPONSE_OK_E);
		// process each`
		dispatch_urc_as_newline_terminated((uint8_t *)urc_processor_hndl.processing_buff, urc_processor_hndl.processing_data_size);
	}else{
		// transparently return
		ret = osSemaphoreAcquire(urc_processor_hndl.write_semaphore, osWaitForever);
		if (ret != osOK) return; // TODO Handle something?
		memset(urc_processor_hndl.curr_read->buff, 0x00, sizeof(urc_processor_hndl.curr_read->buff));
		memset(urc_processor_hndl.curr_write->buff, 0x00, sizeof(urc_processor_hndl.curr_write->buff));
		uart_dequeue_to_buffer((char *)urc_processor_hndl.curr_write->buff, &urc_processor_hndl.curr_write->data_len, internalComm.circularReceiver.rx_buff, &rxQueue);
		urc_processor_hndl.curr_write = urc_processor_hndl.curr_write->next;
		urc_processor_hndl.curr_write->data_len = strlen((char *)urc_processor_hndl.curr_write->buff);
		// Signal TX Thread data is available
		osSemaphoreRelease(urc_processor_hndl.read_semaphore);
		osEventFlagsSet(uart_xfer_event_group, UART_EVENT_TX_INT_DIR_OUT_REQUESTED_E);
	}

}

void process_as_urc(void){
	osStatus_t ret;
	struct uartDataQueue rxQueue;
	//	ret = osMessageQueueGet(externalComm.rxQueue, &rxQueue, 0U, osWaitForever);
	ret = osMessageQueueGet(externalComm.rxQueue, &rxQueue, 0U, osWaitForever);

	// Check if is probably an URC
	bool isNewline = is_uart_queue_data_newline_terminated(externalComm.circularReceiver.rx_buff, &rxQueue);

	if (isNewline){
		// Dequeue to buff
		uart_dequeue_to_buffer((char *)urc_processor_hndl.processing_buff, &urc_processor_hndl.processing_data_size, externalComm.circularReceiver.rx_buff, &rxQueue);

		// process each`
		dispatch_urc_as_newline_terminated((uint8_t *)urc_processor_hndl.processing_buff, urc_processor_hndl.processing_data_size);
	}else{
		// transparently return
		ret = osSemaphoreAcquire(urc_processor_hndl.write_semaphore, osWaitForever);
		if (ret != osOK) return; // TODO Handle something?
		memset(urc_processor_hndl.curr_read->buff, 0x00, sizeof(urc_processor_hndl.curr_read->buff));
		memset(urc_processor_hndl.curr_write->buff, 0x00, sizeof(urc_processor_hndl.curr_write->buff));
		uart_dequeue_to_buffer((char *)urc_processor_hndl.curr_write->buff, &urc_processor_hndl.curr_write->data_len, externalComm.circularReceiver.rx_buff, &rxQueue);
		urc_processor_hndl.curr_write = urc_processor_hndl.curr_write->next;
		urc_processor_hndl.curr_write->data_len = strlen((char *)urc_processor_hndl.curr_write->buff);
		// Signal TX Thread data is available
//		memset(externalComm.circularReceiver.rx_buff, 0x00, UART_BUFFER_SIZE);
		osSemaphoreRelease(urc_processor_hndl.read_semaphore);
		osEventFlagsSet(uart_xfer_event_group, UART_EVENT_TX_EXT_DIR_OUT_REQUESTED_E);
	}

}


void xfer_at_command(struct internal_atc_xfer_s *atc_data){
	osStatus_t  status;
	const char enter[] = {0x0D, 0x0a};
	// Get mutex
	status = osMutexAcquire(atc_xfer_mutex, osWaitForever);

	atc_fsm.atc_handler_cb = atc_data->cb_fn;
	atc_fsm.state = SERCOM_STATE_COMMAND_MODE_E;
//	atc_fsm.state = SERCOM_STATE_BRIDGE_E;

	// send data through internal UART
	status = osSemaphoreAcquire(urc_processor_hndl.write_semaphore, osWaitForever);
	if (status != osOK) return; // TODO Handle something?

	//CHECK IF HAVE DATA ATC
	if (strcmp((char *)atc_data->packet_data, "\0")!=0){
		strcat(atc_data->packet_data, enter);
	}

	//CHECK TYPE ATC
	if (atc_data->atc_mode == ATC_TYPE_WRITE_E){
		urc_processor_hndl.urc_cmp_num = atc_data->cmd_number;
		urc_data.urc_error_check = 0;

	}else{
		urc_data.urc_error_check = 1;
		urc_processor_hndl.urc_cmp_num = atc_data->cmd_number;
	}


	// TODO use predefined port
	memcpy((char *)urc_processor_hndl.curr_write, atc_data->packet_data, (atc_data->packet_len + 2));
	urc_processor_hndl.curr_write->data_len = (atc_data->packet_len + strlen(enter));
	urc_processor_hndl.curr_write = urc_processor_hndl.curr_write->next;


	// Start timer
	status = osTimerStart(atc_xfer_timer, atc_data->timeout_ms);
	osMutexRelease(atc_xfer_mutex);

	// signal to send via uart
	osSemaphoreRelease(urc_processor_hndl.read_semaphore);
	osEventFlagsSet(uart_xfer_event_group, UART_EVENT_TX_INT_DIR_OUT_REQUESTED_E);


	return;
}

void process_as_atc(void){
	return;
}
