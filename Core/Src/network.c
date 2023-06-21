/*
 * network.c
 *
 *  Created on: Apr 10, 2023
 *      Author: admin
 */

#include "network.h"
//#include "printf.h"
#include "led_indicator.h"

const network_change_hook network_changed_hooks[] = {
		NULL,
};

// override network change

int get_sim_status(void){
	return 1;
}

int get_reg_status(void){
	led_indicator_client_set_state(LED_INDICATOR_CLIENT_NETWORK_E, LED_INDICATOR_CLIENT_STATE_NETWORK_REGISTERING_E);
	return 1;
}

int do_datacall(void){
	led_indicator_client_set_state(LED_INDICATOR_CLIENT_NETWORK_E, LED_INDICATOR_CLIENT_STATE_NETWORK_REGISTERING_E);
	return 0;
}

void wait_connection(void){
	led_indicator_client_set_state(LED_INDICATOR_CLIENT_NETWORK_E, LED_INDICATOR_CLIENT_STATE_NETWORK_CONENCTED_2G_E);
	return;
}
