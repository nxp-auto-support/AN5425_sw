/*
 * wdog.c
 *
 *  Created on: Aug 29, 2017
 *      Author: B50982
 */

#include "wdog.h"

void WDOG_disable(void)
{
	WDOG->CNT = 0xD928C520;     /* Unlock watchdog */
	WDOG->TOVAL = 0x0000FFFF;   /* Maximum timeout value */
	WDOG->CS = 0x00002100;      /* Disable watchdog */
}
