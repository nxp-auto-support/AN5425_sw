/*
 * flash.h
 *
 *  Created on: Jan 23, 2017
 *      Author: B50982
 */

#ifndef FLASH_H_
#define FLASH_H_

#include "device_registers.h"

typedef enum {
    eFlash_NoError              = 0,
    eFlash_ReadCollisionError   = 1 << 0,
    eFlash_AccessError          = 1 << 1,
    eFlash_ProtectionViolation  = 1 << 2,
    eFlash_CommandError         = 1 << 3
}eCommandStatus;

eCommandStatus flash_command(uint8_t const *param, uint8_t size);

eCommandStatus flash_check_errors (void);

void flash_clear_errors (eCommandStatus flags);

#endif /* FLASH_H_ */
