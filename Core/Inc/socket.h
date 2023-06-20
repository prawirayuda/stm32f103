/*
 * socket.h
 *
 *  Created on: Jun 20, 2023
 *      Author: muham
 */

#ifndef INC_SOCKET_H_
#define INC_SOCKET_H_


#include "cmsis_os.h"

typedef enum {
	SOCKET_PROTOCOL_TCP_E,
	SOCKET_PROTOCOL_UDP_E
}socket_protocol_e;

typedef enum {
	SOCKET_MODE_SERVER_E,
	SOCKET_MODE_CLIENT_E
}socket_mode_e;

typedef enum {
	POWER_LOST_FLAGE_FALSE,
	POWER_LOST_FLAG_TRUE
}powerLost_flag_e;

//typedef enum {
//	SOCKET_CONNECTED_E = 1<<0,
//	SOCKET_CONNECTED_AS_TCPCLIENT_E = 1<<1,
//	SOCKET_NOT_CONNECTED_E
//}socket_connection_flag_e;

typedef enum {
	SOCKET_STATE_INITIAL_E,
	SOCKET_STATE_BOOT_E,
	SOCKET_STATE_IDLE_E,
	SOCKET_STATE_DISCONNECTED_E,
	SOCKET_STATE_CONNECTED_E,
	SOCKET_STATE_BUSY_E
};

typedef enum {
	SOCKET_CONECTION_PROTOCOL_CHANGE_E = 1 << 0,
	SOCKET_CONNECTION_MODE_CHANGE_E = 1 << 1,
	SOCKET_CONNECTION_DISCONNECTED_E = 1 << 2,
	SOCKET_CONNECTION_CONNECTED_E = 1 << 3,
	SOCKET_CONNECTION_TRANSMISSION_DONE_E = 1 << 4
}socket_connection_signal_e;

void socket_init(void);


#endif /* INC_SOCKET_H_ */
