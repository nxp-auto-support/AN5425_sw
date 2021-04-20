/*
 * lpit.h
 *
 *  Created on: Apr 8, 2016
 *      Author: B46911
 */

#ifndef LPTMR_H_
#define LPTMR_H_

#include "device_registers.h" /* include peripheral declarations */

typedef void (*vfcn_callback)(void);

void LPTMR_init(uint32_t ms, vfcn_callback callback);

#endif /* LPTMR_H_ */
