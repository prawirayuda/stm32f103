/*
 * led_indicator.c
 *
 *  Created on: Jun 20, 2023
 *      Author: muham
 */



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>
#include "cmsis_os.h"
#include "main.h"
#include "atc_processor.h"
#include "serial_communication.h"
#include "led_indicator.h"

struct led_pattern_list *construct_state_pattern_list(
#ifndef LED_INDICATOR_USE_DYNAMIC_ALLOCATION
		struct led_pattern_list *led_pattern_mem,
#endif
		uint8_t no_of_pattern_list,
		...
);

void led_indicator_timer_callback(void *args);
unsigned int getFirstSetBitPos(int n);

/* Start of Application Level */
#define NO_OF_CLIENTS 3

// Power Patterns
#define NO_OF_POWER_STATE							3
#define	NO_OF_PATTERN_IN_POWER_STATE_OFF			1
#define NO_OF_PATTERN_IN_POWER_STATE_ON				1
#define NO_OF_PATTERN_IN_POWER_STATE_BACKUP			2
#define NO_OF_PATTERN_IN_POWER_STATE_TOTAL			NO_OF_PATTERN_IN_POWER_STATE_OFF + \
													NO_OF_PATTERN_IN_POWER_STATE_ON + \
													NO_OF_PATTERN_IN_POWER_STATE_BACKUP

// Network Registration Patterns
#define NO_OF_NETWORK_STATE 						6
#define NO_OF_PATTERN_IN_NETWORK_STATE_IDLE_OFF		1
#define NO_OF_PATTERN_IN_NETWORK_STATE_REGISTERING	2
#define NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_2G	2
#define NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_3G	2
#define NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_4G	2
#define NO_OF_PATTERN_IN_NETWORK_STATE_ERROR		1
#define NO_OF_PATTERN_IN_NETWORK_STATE_TOTAL		NO_OF_PATTERN_IN_NETWORK_STATE_IDLE_OFF + \
													NO_OF_PATTERN_IN_NETWORK_STATE_IDLE_OFF + \
													NO_OF_PATTERN_IN_NETWORK_STATE_REGISTERING + \
													NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_2G + \
													NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_3G + \
													NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_4G + \
													NO_OF_PATTERN_IN_NETWORK_STATE_ERROR

// Data Connection Status Patterns
#define NO_OF_STATUS_STATE 							3
#define NO_OF_PATTERN_IN_STATUS_STATE_NO_DATA		1
#define NO_OF_PATTERN_IN_STATUS_STATE_DATA_CONN		1
#define NO_OF_PATTERN_IN_STATUS_STATE_TRANSMITTING	2
#define NO_OF_PATTERN_IN_STATUS_STATE_TOTAL			NO_OF_PATTERN_IN_STATUS_STATE_NO_DATA + \
													NO_OF_PATTERN_IN_STATUS_STATE_DATA_CONN + \
													NO_OF_PATTERN_IN_STATUS_STATE_TRANSMITTING

extern struct tsm_UARTHandle externalComm;
extern struct tsm_UARTHandle internalComm;
extern osEventFlagsId_t uart_xfer_event_group;
extern processor_handle_t urc_processor_hndl;

// TODO event flag creation might fail and return NULL
static osEventFlagsId_t led_indicator_event_id;

// Memories
struct led_pattern_list *power_state_patterns[NO_OF_POWER_STATE];
struct led_pattern_list power_state_pattern_mem[NO_OF_PATTERN_IN_POWER_STATE_TOTAL];

struct led_pattern_list *network_state_patterns[NO_OF_NETWORK_STATE];
struct led_pattern_list network_state_pattern_mem[NO_OF_PATTERN_IN_NETWORK_STATE_TOTAL];

struct led_pattern_list *status_state_patterns[NO_OF_STATUS_STATE];
struct led_pattern_list status_state_pattern_mem[NO_OF_PATTERN_IN_STATUS_STATE_TOTAL];

static struct led_indicator_client led_indicator_clients[NO_OF_CLIENTS] = {
		{
				.client_name = "power",
				.GPIOx = LED_PWR_GPIO_Port,
				.GPIO_Pin = LED_PWR_Pin,
				.no_of_client_state = NO_OF_POWER_STATE,
				.flag_bit = 0x1U << 0
		},
		{
				.client_name = "network",
				.GPIOx = LED_NW_GPIO_Port,
				.GPIO_Pin = LED_NW_Pin,
				.no_of_client_state = NO_OF_NETWORK_STATE,
				.flag_bit = 0x1U << 1
		},
		{
				.client_name = "status",
				.GPIOx = LED_DATA_GPIO_Port,
				.GPIO_Pin = LED_DATA_Pin,
				.no_of_client_state = NO_OF_STATUS_STATE,
				.flag_bit = 0x1U << 2
		}
};

void network_tech_changed_indication(uint8_t *data, uint16_t len){
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
	return;
}

static void led_indicator_init(void){
	// OS Component Initialize
		led_indicator_event_id = osEventFlagsNew(NULL);

		// Initialize Power Client

		power_state_patterns[LED_INDICATOR_CLIENT_STATE_POWER_OFF_E] = construct_state_pattern_list(
			&power_state_pattern_mem[LED_INDICATOR_CLIENT_STATE_POWER_OFF_E],
			NO_OF_PATTERN_IN_POWER_STATE_OFF,
			(struct led_pattern){0, 500}
		);

		power_state_patterns[LED_INDICATOR_CLIENT_STATE_POWER_ON_E] = construct_state_pattern_list(
			&power_state_pattern_mem[LED_INDICATOR_CLIENT_STATE_POWER_OFF_E] + \
				NO_OF_PATTERN_IN_POWER_STATE_OFF,
			NO_OF_PATTERN_IN_POWER_STATE_ON,
			(struct led_pattern){1, 500}
		);

		power_state_patterns[LED_INDICATOR_CLIENT_STATE_POWER_BACKUP_E] = construct_state_pattern_list(
			&(power_state_pattern_mem[LED_INDICATOR_CLIENT_STATE_POWER_OFF_E]) + \
				NO_OF_PATTERN_IN_POWER_STATE_OFF + \
				NO_OF_PATTERN_IN_POWER_STATE_ON,
			NO_OF_PATTERN_IN_POWER_STATE_BACKUP,
			(struct led_pattern){1, 500},
			(struct led_pattern){0, 2500}
		);

		// Populate Clients' Handle
		led_indicator_clients[LED_INDICATOR_CLIENT_POWER_E].client_state_data = power_state_patterns;
		led_indicator_clients[LED_INDICATOR_CLIENT_POWER_E].curr_led_pattern = \
				power_state_patterns[LED_INDICATOR_CLIENT_STATE_POWER_OFF_E]; // always initialize to 0
		led_indicator_clients[LED_INDICATOR_CLIENT_POWER_E].curr_client_state = \
				LED_INDICATOR_CLIENT_STATE_POWER_OFF_E;

		/********************************************************/

		// Initialize Network Client
		network_state_patterns[LED_INDICATOR_CLIENT_STATE_NETWORK_IDLE_OFF_E] = \
				construct_state_pattern_list(
						&network_state_pattern_mem[LED_INDICATOR_CLIENT_STATE_NETWORK_IDLE_OFF_E],
			NO_OF_PATTERN_IN_NETWORK_STATE_IDLE_OFF,
			(struct led_pattern){0, 500}
		);

		network_state_patterns[LED_INDICATOR_CLIENT_STATE_NETWORK_REGISTERING_E] = \
				construct_state_pattern_list(
						&(network_state_pattern_mem[LED_INDICATOR_CLIENT_STATE_NETWORK_IDLE_OFF_E]) + \
						NO_OF_PATTERN_IN_NETWORK_STATE_IDLE_OFF,
			NO_OF_PATTERN_IN_NETWORK_STATE_REGISTERING,
			(struct led_pattern){1, 500},
			(struct led_pattern){0, 500}
		);

		network_state_patterns[LED_INDICATOR_CLIENT_STATE_NETWORK_CONENCTED_2G_E] = \
				construct_state_pattern_list(
						&(network_state_pattern_mem[0]) + \
				NO_OF_PATTERN_IN_NETWORK_STATE_IDLE_OFF + \
				NO_OF_PATTERN_IN_NETWORK_STATE_REGISTERING,
			NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_2G,
			(struct led_pattern){1, 500},
			(struct led_pattern){0, 2500}
		);

		network_state_patterns[LED_INDICATOR_CLIENT_STATE_NETWORK_CONNECTED_3G_E] = construct_state_pattern_list(
			&(network_state_pattern_mem[LED_INDICATOR_CLIENT_STATE_NETWORK_IDLE_OFF_E]) + \
				NO_OF_PATTERN_IN_NETWORK_STATE_IDLE_OFF + \
				NO_OF_PATTERN_IN_NETWORK_STATE_REGISTERING + \
				NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_2G,
			NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_3G,
			(struct led_pattern){1, 1000},
			(struct led_pattern){0, 2000}
		);

		network_state_patterns[LED_INDICATOR_CLIENT_STATE_NETWORK_CONNECTED_4G_E] = construct_state_pattern_list(
			&(network_state_pattern_mem[LED_INDICATOR_CLIENT_STATE_NETWORK_IDLE_OFF_E]) + \
				NO_OF_PATTERN_IN_NETWORK_STATE_IDLE_OFF + \
				NO_OF_PATTERN_IN_NETWORK_STATE_REGISTERING + \
				NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_2G + \
				NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_3G,
			NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_4G,
			(struct led_pattern){1, 1500},
			(struct led_pattern){0, 2500}
		);

		network_state_patterns[LED_INDICATOR_CLIENT_STATE_NETWORK_ERROR_E] = construct_state_pattern_list(
				&(network_state_pattern_mem[LED_INDICATOR_CLIENT_STATE_NETWORK_IDLE_OFF_E]) + \
					NO_OF_PATTERN_IN_NETWORK_STATE_IDLE_OFF + \
					NO_OF_PATTERN_IN_NETWORK_STATE_REGISTERING + \
					NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_2G + \
					NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_3G + \
					NO_OF_PATTERN_IN_NETWORK_STATE_CONNECTED_4G,
				NO_OF_PATTERN_IN_NETWORK_STATE_ERROR,
				(struct led_pattern){1, 1000}
			);

		// Populate Clients' Handle
		led_indicator_clients[LED_INDICATOR_CLIENT_NETWORK_E].client_state_data = network_state_patterns;
		led_indicator_clients[LED_INDICATOR_CLIENT_NETWORK_E].curr_led_pattern = network_state_patterns[LED_INDICATOR_CLIENT_STATE_NETWORK_IDLE_OFF_E]; // always initialize to 0
		led_indicator_clients[LED_INDICATOR_CLIENT_NETWORK_E].curr_client_state = LED_INDICATOR_CLIENT_STATE_NETWORK_IDLE_OFF_E;

		/********************************************************/

		// Initialize Status Client
		status_state_patterns[LED_INDICATOR_CLIENT_STATE_STATUS_NO_DATA_E] = construct_state_pattern_list(
			&status_state_pattern_mem[LED_INDICATOR_CLIENT_STATE_STATUS_NO_DATA_E],
			NO_OF_PATTERN_IN_STATUS_STATE_NO_DATA,
			(struct led_pattern){0, 500}
		);

		status_state_patterns[LED_INDICATOR_CLIENT_STATE_STATUS_DATA_CONN_E] = construct_state_pattern_list(
			&(status_state_pattern_mem[LED_INDICATOR_CLIENT_STATE_STATUS_NO_DATA_E]) + \
				NO_OF_PATTERN_IN_STATUS_STATE_NO_DATA,
				NO_OF_PATTERN_IN_STATUS_STATE_DATA_CONN,
			(struct led_pattern){1, 500}
		);

		status_state_patterns[LED_INDICATOR_CLIENT_STATE_STATUS_DATA_TRANSMITTING_E] = construct_state_pattern_list(
			&(status_state_pattern_mem[LED_INDICATOR_CLIENT_STATE_STATUS_NO_DATA_E]) + \
				NO_OF_PATTERN_IN_STATUS_STATE_NO_DATA + \
				NO_OF_PATTERN_IN_STATUS_STATE_DATA_CONN,
			NO_OF_PATTERN_IN_STATUS_STATE_TRANSMITTING,
			(struct led_pattern){1, 500},
			(struct led_pattern){0, 500}
		);


		// Populate Clients' Handle
		led_indicator_clients[LED_INDICATOR_CLIENT_STATUS_E].client_state_data = status_state_patterns;
		led_indicator_clients[LED_INDICATOR_CLIENT_STATUS_E].curr_led_pattern = status_state_patterns[LED_INDICATOR_CLIENT_STATE_STATUS_NO_DATA_E]; // always initialize to 0
		led_indicator_clients[LED_INDICATOR_CLIENT_STATUS_E].curr_client_state = LED_INDICATOR_CLIENT_STATE_STATUS_NO_DATA_E;

		/********************************************************/

		for (uint8_t i = 0; i < NO_OF_CLIENTS; i++){
			// Initialize GPIOs;
			HAL_GPIO_Init(
				led_indicator_clients[i].GPIOx,
				&(GPIO_InitTypeDef){
					.Pin = led_indicator_clients[i].GPIO_Pin,
					.Mode = GPIO_MODE_OUTPUT_PP,
					.Pull = GPIO_NOPULL,
					.Speed = GPIO_SPEED_FREQ_HIGH
				}
			);

			// Create timer
			led_indicator_clients[i].timer_id = osTimerNew(led_indicator_timer_callback, osTimerOnce, &led_indicator_clients[i].flag_bit, NULL);
			// Set initial led state
			HAL_GPIO_WritePin(led_indicator_clients[i].GPIOx, led_indicator_clients[i].GPIO_Pin, led_indicator_clients[i].curr_led_pattern->pattern.level);
			// Start Timer
			osTimerStart(led_indicator_clients[i].timer_id, led_indicator_clients[i].curr_led_pattern->pattern.period_ms);
		}
}

void led_indicator_task(void *argument){
	uint32_t resultFlag = 0x0000;
	uint32_t flagMask = 0x07;
	uint8_t firstSetBitPos;
	led_indicator_init();

	while (1){
		resultFlag = osEventFlagsWait(led_indicator_event_id, flagMask, osFlagsWaitAny, osWaitForever);
		while (resultFlag){
			firstSetBitPos = getFirstSetBitPos(resultFlag) - 1;
			led_indicator_clients[firstSetBitPos].curr_led_pattern = led_indicator_clients[firstSetBitPos].curr_led_pattern->next;
			HAL_GPIO_WritePin(led_indicator_clients[firstSetBitPos].GPIOx, led_indicator_clients[firstSetBitPos].GPIO_Pin, led_indicator_clients[firstSetBitPos].curr_led_pattern->pattern.level);
			osTimerStart(led_indicator_clients[firstSetBitPos].timer_id, led_indicator_clients[firstSetBitPos].curr_led_pattern->pattern.period_ms);
			resultFlag &= ~(0x1U << firstSetBitPos);
		}
	}
}

/* End of Application Level */

/*		---------- End of Application Level Code ----------
		------- You should not edit anything below --------*/

struct led_pattern_list *construct_state_pattern_list(
#ifndef LED_INDICATOR_USE_DYNAMIC_ALLOCATION
		struct led_pattern_list *led_pattern_mem,
#endif
		uint8_t no_of_pattern_list,
		...
){
	va_list pattern_list;
	struct led_pattern curr_arg;
#ifdef LED_INDICATOR_USE_DYNAMIC_ALLOCATION
	struct led_pattern_list *head = malloc(no_of_pattern_list * sizeof(struct led_pattern_list));
#else
	struct led_pattern_list *head = led_pattern_mem;
#endif
	struct led_pattern_list *curr = head;

	va_start(pattern_list, no_of_pattern_list);

	for (int i = 0; i < no_of_pattern_list; i++){
		curr_arg = va_arg(pattern_list, struct led_pattern);
		curr->pattern.level = curr_arg.level;
		curr->pattern.period_ms = curr_arg.period_ms;
		if (i < no_of_pattern_list - 1){
			curr->next = (void *)curr + sizeof(struct led_pattern_list);
		curr = curr->next;
		}else{
			curr->next = head;
		}
	}

	va_end(pattern_list);

	return head;
}

uint8_t led_indicator_client_set_state(
#ifdef LED_INDICATOR_CLIENT_IS_HANDLE
		void			*handle,
#elif defined(LED_INDICATOR_CLIENT_IS_ID)
		uint8_t		client_id,
#endif
		uint8_t		desired_state
){
	// Sanitize input
	if (client_id > NO_OF_CLIENTS - 1){ // Client ID is 0 indexed
		return LED_INDICATOR_SET_STATE_ERR_INVALID_CLIENT_E;
	}

	if (desired_state >= led_indicator_clients[client_id].no_of_client_state){ // Client state is 0 indexed
		return LED_INDICATOR_SET_STATE_ERR_INVALID_STATE_ID_E;
	}

	// Stop timer
	osTimerStop(led_indicator_clients[client_id].timer_id);

	// Set current state to desired state and reset pattern pointer
	led_indicator_clients[client_id].curr_client_state = desired_state;
	led_indicator_clients[client_id].curr_led_pattern = \
			led_indicator_clients[client_id].client_state_data[desired_state];

	// Write GPIO Level
	HAL_GPIO_WritePin(
		led_indicator_clients[client_id].GPIOx,
		led_indicator_clients[client_id].GPIO_Pin,
		led_indicator_clients[client_id].curr_led_pattern->pattern.level
	);

	// Start Timer Again
	osTimerStart(
		led_indicator_clients[client_id].timer_id,
		led_indicator_clients[client_id].curr_led_pattern->pattern.period_ms
	);

	return LED_INDICATOR_SET_STATE_ERR_NO_ERR_OK_E;
}

void led_indicator_timer_callback(void *args){
	osEventFlagsSet(led_indicator_event_id, *(uint32_t *)args);
	osThreadYield();
}

// helper function to get the first set bit
unsigned int getFirstSetBitPos(int n){
	return log2(n & -n) + 1;
}
