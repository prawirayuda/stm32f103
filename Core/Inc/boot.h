/*
 * boot.h
 *
 *  Created on: Jun 20, 2023
 *      Author: muham
 */

#ifndef INC_BOOT_H_
#define INC_BOOT_H_


#include "atc_processor.h"
#include "cmsis_os.h"

//enum {
//	BOOT_SEQ_ATC_CALL_COMPLETED_E 	= 1 << 0,
//	BOOT_SEQ_ATC_CALL_TIMEOUT_E 	= 1 << 1,
//	BOOT_SEQ_COMPLETED_E 			= 1 << 2,
//	BOOT_SEQ_REQUESTED_E 			= 1 << 3,
//	BOOT_SEQ_INVALID_E
//};



void boot_init(void *args);
void wait_boot(void);

#endif /* INC_BOOT_H_ */
