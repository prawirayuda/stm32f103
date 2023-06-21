/*
 * led_indicator.h
 *
 *  Created on: Jan 17, 2023
 *      Author: admin
 */

#ifndef INC_LED_INDICATOR_H_
#define INC_LED_INDICATOR_H_

/* GPIO Style switch */
#define GPIO_STYLE_STM_HAL
//#define GPIO_STYLE_INTEGER

/* Memory Allocation Dyamic/Static Switch */
//#define LED_INDICATOR_USE_DYNAMIC_ALLOCATION

/* LED Indicator Client Identifier Style */
//#define LED_INDICATOR_CLIENT_IS_HANDLE
#define LED_INDICATOR_CLIENT_IS_ID

#define PROJECT_USE_CMSIS_RTOS

#ifdef PROJECT_USE_CMSIS_RTOS
#include "cmsis_os.h"

// Timer Name wrapper
#define TSM_toTimerName(a, b) (b ## a)
#endif

#ifdef GPIO_STYLE_STM_HAL
#include "stm32f1xx_hal.h"
#endif

/* Start of Application Level */
typedef enum {
	LED_INDICATOR_CLIENT_POWER_E,
	LED_INDICATOR_CLIENT_NETWORK_E,
	LED_INDICATOR_CLIENT_STATUS_E,
	LED_INDICATOR_CLIENT_INVALID_E,
	LED_INDICATOR_CLIENT_MAX 			= LED_INDICATOR_CLIENT_INVALID_E - 1
}led_indicator_client_id_e;

typedef enum {
	LED_INDICATOR_CLIENT_STATE_POWER_OFF_E,
	LED_INDICATOR_CLIENT_STATE_POWER_ON_E,
	LED_INDICATOR_CLIENT_STATE_POWER_BACKUP_E,
	LED_INDICATOR_CLIENT_STATE_POWER_INVALID_E,
	LED_INDICATOR_CLIENT_STATE_POWER_MAX = LED_INDICATOR_CLIENT_STATE_POWER_INVALID_E - 1
}led_indicator_client_state_power_e;

typedef enum {
	LED_INDICATOR_CLIENT_STATE_NETWORK_IDLE_OFF_E,
	LED_INDICATOR_CLIENT_STATE_NETWORK_REGISTERING_E,
	LED_INDICATOR_CLIENT_STATE_NETWORK_CONENCTED_2G_E,
	LED_INDICATOR_CLIENT_STATE_NETWORK_CONNECTED_3G_E,
	LED_INDICATOR_CLIENT_STATE_NETWORK_CONNECTED_4G_E,
	LED_INDICATOR_CLIENT_STATE_NETWORK_ERROR_E,
	LED_INDICATOR_CLIENT_STATE_NETWORK_INVALID_E,
	LED_INDICATOR_CLIENT_STATE_NETWORK_MAX = LED_INDICATOR_CLIENT_STATE_NETWORK_INVALID_E - 1
}led_indicator_client_state_network_e;

typedef enum{
	LED_INDICATOR_CLIENT_STATE_STATUS_NO_DATA_E,
	LED_INDICATOR_CLIENT_STATE_STATUS_DATA_CONN_E,
	LED_INDICATOR_CLIENT_STATE_STATUS_DATA_TRANSMITTING_E,
	LED_INDICATOR_CLIENT_STATE_STATUS_INVALID_E,
	LED_INDICATOR_CLIENT_STATE_STATUS_MAX = LED_INDICATOR_CLIENT_STATE_STATUS_INVALID_E - 1
}led_indicator_client_state_status_e;
/* End of Application Level */

typedef enum{
	LED_INDICATOR_SET_STATE_ERR_NO_ERR_OK_E,
	LED_INDICATOR_SET_STATE_ERR_INVALID_CLIENT_E,
	LED_INDICATOR_SET_STATE_ERR_INVALID_STATE_ID_E
}led_indicator_set_state_err;;

struct led_pattern{
	uint8_t		level;
	uint16_t	period_ms;
};

struct led_pattern_list{
	struct led_pattern 		pattern;
	struct led_pattern_list *next;
};

struct led_indicator_client{
	char					client_name[16];
#ifdef GPIO_STYLE_STM_HAL
	GPIO_TypeDef			*GPIOx;
	uint16_t				GPIO_Pin;
#else
    uint8_t   gpio;
#endif
#ifdef PROJECT_USE_CMSIS_RTOS
    osTimerId				timer_id;
    osTimerDef_t			timer_def;
#else
    void					*timer;
#endif
    uint32_t				flag_bit;
    uint8_t   				curr_client_state;
    uint8_t					no_of_client_state;
    struct led_pattern_list	**client_state_data;
    struct led_pattern_list *curr_led_pattern;
};

//void led_indicator_task(void *argument);
void led_task_init(void *args);
uint8_t led_indicator_client_set_state(
#ifdef LED_INDICATOR_CLIENT_IS_HANDLE
		void			*handle,
#elif defined(LED_INDICATOR_CLIENT_IS_ID)
		uint8_t		client_id,
#endif
		uint8_t		desired_state
);
#endif /* INC_LED_INDICATOR_H_ */
