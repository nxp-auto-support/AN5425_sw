/*
 * lowPower.h
 *
 *  Created on: Aug 29, 2017
 *      Author: B50982
 */

#ifndef POWERMANAGEMENT_H_
#define POWERMANAGEMENT_H_

#include "device_registers.h"

typedef void (*vfcn_callback)(void);

void Run_to_HSRUN (vfcn_callback beforeCb, vfcn_callback afterCb);
void HSRUN_to_RUN (vfcn_callback beforeCb, vfcn_callback afterCb);

#endif /* POWERMANAGEMENT_H_ */
