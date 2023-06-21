/*
 * network.h
 *
 *  Created on: Apr 10, 2023
 *      Author: admin
 */

#ifndef INC_NETWORK_H_
#define INC_NETWORK_H_

#include "cmsis_os.h"

typedef void (*network_change_hook)(void);

int get_sim_status(void);
int get_reg_status(void);
void wait_connection(void);

#endif /* INC_NETWORK_H_ */
