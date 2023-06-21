/*
 * serial_communication.h
 *
 *  Created on: Jun 20, 2023
 *      Author: muham
 */

#ifndef INC_SERIAL_COMMUNICATION_H_
#define INC_SERIAL_COMMUNICATION_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#define UART_BUFFER_SIZE 128

extern UART_HandleTypeDef huart2;
#define EXTERNAL_UART_HANDLE huart2

#define USE_INTERNAL_UART 3

#ifdef USE_INTERNAL_UART
extern UART_HandleTypeDef huart3;
#define INTERNAL_UART_HANDLE huart3
#endif

extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart3_rx;

typedef enum{
	SERCOM_STATE_IDLE_E,
	SERCOM_STATE_COMMAND_MODE_E,
	SERCOM_STATE_DATA_MODE_E,
	SERCOM_STATE_PASSTHROUGH_E,
	SERCOM_STATE_BRIDGE_E,
	SERCOM_STATE_INVALID_E
}sercom_state_e;

struct sercomStateMachine{
	sercom_state_e		state;
	void 				*userData;
};

struct circularUARTReceiver{
	// Head and tail start from 0
	uint16_t 	head_pos;
	uint16_t	tail_pos;
	char 		rx_buff[UART_BUFFER_SIZE];
};

struct tsm_UARTHandle{
	UART_HandleTypeDef			huart;
	char 						tx_buff[UART_BUFFER_SIZE];
	uint32_t					state;
	osMessageQueueId_t			rxQueue;
	osMessageQueueId_t			txQueue;
	struct circularUARTReceiver circularReceiver;
	void						(*onRxCb)(void *);
	void						(*onTxCb)(void *);
};

struct uartDataQueue{
	uint16_t 	data_start_pos;
	uint16_t 	len;
};

typedef enum {
	UART_EVENT_TX_EXT_DIR_OUT_REQUESTED_E 		= 1 << 0,
	UART_EVENT_TX_INT_DIR_OUT_REQUESTED_E 		= 1 << 1,
	UART_EXT_EVENT_DMA_TX_COMPLETE 			= 1 << 2
}uart_xfer_event_e;

static __inline void uart_dequeue_to_buffer(char *dest_buff, uint16_t *data_len, char *source_buf, struct uartDataQueue *queue_data){
	size_t first_part_len = queue_data->len + queue_data->data_start_pos > UART_BUFFER_SIZE
			? UART_BUFFER_SIZE - queue_data->data_start_pos : queue_data->len;
	memcpy(
			dest_buff,
			source_buf + queue_data->data_start_pos,
			first_part_len
	);
	*data_len += first_part_len;

	if (queue_data->len + queue_data->data_start_pos > UART_BUFFER_SIZE){
		memcpy(
			dest_buff + first_part_len,
			source_buf,
			queue_data->len - (UART_BUFFER_SIZE - queue_data->data_start_pos)
		);
		*data_len += queue_data->len - (UART_BUFFER_SIZE - queue_data->data_start_pos);
	}
}

static __inline bool is_uart_queue_data_newline_terminated(char *source_buf, struct uartDataQueue *queue_data){
	uint16_t last_char_pos = queue_data->data_start_pos + queue_data->len > UART_BUFFER_SIZE ? \
			queue_data->len - (UART_BUFFER_SIZE - queue_data->data_start_pos) - 1 : queue_data->data_start_pos + queue_data->len - 1;

	return source_buf[last_char_pos] == '\n' || source_buf[last_char_pos] == '\r';
}

void TSM_UART_IRQ_Handler(UART_HandleTypeDef *huart);
void sercomm_init(void *args);




#endif /* INC_SERIAL_COMMUNICATION_H_ */
