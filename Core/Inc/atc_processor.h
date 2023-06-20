/*
 * atc_processor.h
 *
 *  Created on: Jun 20, 2023
 *      Author: muham
 */

#ifndef INC_ATC_PROCESSOR_H_
#define INC_ATC_PROCESSOR_H_

#include "cmsis_os.h"

#define UART_MAX_FRAME_SIZE	512
//#define PROCESSING_BUFF_SIZE 512
#define PROCESSING_BUFF_SIZE 512


typedef enum {
	ATC_XFER_COMPLETED_E 	= 1 << 0,
	ATC_XFER_TIMEOUT_E 		= 1 << 1,
	ATC_XFER_INVALID_E		= 1 << 2
}atc_xfer_progress_e;

enum {
	BOOT_SEQ_ATC_CALL_COMPLETED_E 	= 1 << 0,
	BOOT_SEQ_ATC_CALL_TIMEOUT_E 	= 1 << 1,
	BOOT_SEQ_COMPLETED_E 			= 1 << 2,
	BOOT_SEQ_REQUESTED_E 			= 1 << 3,
	BOOT_SEQ_INVALID_E
};

typedef enum {
	SOCKET_CONNECTED_E 		= 1<<0,
	SOCKET_NOT_CONNECTED_E 	= 1<<1,
	SOCKET_NOT_INVALID_E 	= 1<<2
}socket_connection_flag_e;
typedef enum {
	BOOT_EVENT_REQUESTED_E 			= 1 << 0,
	BOOt_EVENT_COMPLETE 			= 1 << 1
}boot_event_e;

typedef enum {
	ATC_TYPE_WRITE_E = 1<<0,
	ATC_TYPE_READ_E = 1<<1
}atc_cmd_type_e;

typedef enum {
	URC_ERROR_true,
	URC_ERROR_false
}urc_err_state;

//typedef struct {
//	atc_cmd_type_e cmd_type;
//	int cmd_number;
//} cmd_mode_s;

typedef struct {
	uint8_t						state;
	uint8_t						feature_id;
	int							(*atc_handler_cb)(uint8_t *, uint16_t);
}atc_processor_runtime_fsm_t;

struct atc_xfer_req_s{
	void	(*cb_fn)(uint8_t *, uint16_t);
	char 	keyword[55]; // subject to change
//	const char (*keyword)(uint8_t *);
	uint8_t	keyword_len;
	int urc_read_nmbr;
};

struct internal_atc_xfer_s{
	int 		(*cb_fn)(uint8_t *, uint16_t);
	char		packet_data[255];
	uint16_t	packet_len;
	uint16_t	timeout_ms;
	atc_cmd_type_e atc_mode;
	int cmd_number;
};

typedef struct urc_expected_data{
	char		 packet_urc[255];
	uint16_t	 urc_len;
	uint16_t	 urc_ok;
	uint16_t	 urc_error_check;
	int 		 connectivity_check;
}urc_expected_data_s;

typedef struct connectivity{
	int get_connectivity;
	char client_number[5];
	char ip[5];
}connectivity_urc_s;

typedef struct double_buffered_buff_s{
	uint8_t 							buff[UART_MAX_FRAME_SIZE];
	uint16_t							data_len;
	struct double_buffered_buff_s	*next;
}double_buffered_buff_t;

typedef struct processor_handle_s{
	osSemaphoreId_t write_semaphore;
	osSemaphoreId_t read_semaphore;
	uint8_t					processing_buff[PROCESSING_BUFF_SIZE];
	uint16_t				processing_data_size;
	double_buffered_buff_t	*curr_read;
	double_buffered_buff_t	*curr_write;
	double_buffered_buff_t buff[2];
	int			   urc_cmp_num;
}processor_handle_t;

void data_bridge_dir_out_notify(void);
void xfer_at_command(struct internal_atc_xfer_s *);
void process_as_urc(void);
void internal_receive_urc(void);
void atc_init(void);
void reboot(void);
#endif /* INC_ATC_PROCESSOR_H_ */
