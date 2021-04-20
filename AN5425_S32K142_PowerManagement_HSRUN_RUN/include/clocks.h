/*
 * clocks.h
 *
 *  Created on: Jan 9, 2018
 *      Author: B50982
 */

#ifndef CLOCKS_H_
#define CLOCKS_H_

#include "device_registers.h"

void Clock_init (void);

uint8_t Clock_run_configuration (void);

uint8_t Clock_hsrun_configuration (void);

#endif /* CLOCKS_H_ */
