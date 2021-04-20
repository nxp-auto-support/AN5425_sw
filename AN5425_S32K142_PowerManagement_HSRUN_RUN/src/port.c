/*
 * port.c
 *
 *  Created on: Apr 27, 2018
 *      Author: nxa12021
 */

#include "port.h"


void PORT_init (void)
{
	PCC->PCCn[PCC_PORTC_INDEX ]|=PCC_PCCn_CGC_MASK; /* Enable clock for PORTC */
	PORTC->PCR[6]|=PORT_PCR_MUX(2);  /* Port C6:  MUX = ALT2,UART2 TX */
	PORTC->PCR[7]|=PORT_PCR_MUX(2);  /* Port C7:  MUX = ALT2,UART2 RX */

    /* Initialize PIN to signal power mode transitions */
    PCC->PCCn[PWR_MODE_PORT_PCC_INDEX] |= PCC_PCCn_CGC_MASK;
    PWR_MODE_PORT_PTR->PCR[PWR_MODE_GPIO_PIN] = PORT_PCR_MUX(1);
    PWR_MODE_GPIO_PTR->PSOR |= 1 << PWR_MODE_GPIO_PIN;
    PWR_MODE_GPIO_PTR->PDDR |= 1 << PWR_MODE_GPIO_PIN;

    /* Initialize PIN to signal PDB period */
    PCC->PCCn[PDB_PORT_PCC_INDEX] |= PCC_PCCn_CGC_MASK;
    PDB_PORT_PTR->PCR[PDB_GPIO_PIN] = PORT_PCR_MUX(1);
    PDB_GPIO_PTR->PCOR |= 1 << PDB_GPIO_PIN;
    PDB_GPIO_PTR->PDDR |= 1 << PDB_GPIO_PIN;

    /* CLKOUT - SysClock */
    PCC->PCCn[PCC_PORTB_INDEX] |= PCC_PCCn_CGC_MASK;
    PORTB->PCR[5] = PORT_PCR_MUX(5);

    SIM->CHIPCTL &= ~(SIM_CHIPCTL_CLKOUTEN_MASK |
                    SIM_CHIPCTL_CLKOUTDIV_MASK |
                    SIM_CHIPCTL_CLKOUTSEL_MASK);
    SIM->CHIPCTL |= SIM_CHIPCTL_CLKOUTSEL(7) |  /* HCLK - SYS_CLOCK */
                    SIM_CHIPCTL_CLKOUTDIV(7);   /* Divided by 8 */
    SIM->CHIPCTL |= SIM_CHIPCTL_CLKOUTEN_MASK;

}
