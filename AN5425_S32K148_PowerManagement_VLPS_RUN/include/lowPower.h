/*
 * lowPower.h
 *
 *  Created on: Aug 29, 2017
 *      Author: B50982
 */

#ifndef LOWPOWER_H_
#define LOWPOWER_H_

#include "device_registers.h"

typedef enum _mode {
	eRun 	= 1,
	eStop	= 2,
	eVLPR	= 4,
	eVLPS   = 16,
	eHSRun	= 128
}eLowPowerMode;

void Run_to_VLPS (void);
void Run_to_VLPR (void);
void VLPR_to_VLPS (void);

#endif /* LOWPOWER_H_ */
