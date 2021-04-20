/*
 * lowPower.c
 *
 *  Created on: Aug 29, 2017
 *      Author: B50982
 */

#include "PowerManagement.h"
#include "clocks.h"

#ifndef NULL
#define NULL    (void *)0
#endif

typedef enum _mode {
    eRun    = 1,
    eStop   = 2,
    eVLPR   = 4,
    eVLPS   = 16,
    eHSRun  = 128
}ePowerMode;

void Run_to_HSRUN (vfcn_callback beforeCb, vfcn_callback afterCb)
{
    /* Configure clocks before making the transisiton */
    Clock_hsrun_configuration();

    /* Allow high speed run mode */
    SMC->PMPROT |= SMC_PMPROT_AHSRUN_MASK;

    /* Check if current mode is RUN mode */
    if(eRun == SMC->PMSTAT)
    {

        if (NULL != beforeCb)
        {
            /* Execute callback before doing the transition */
            beforeCb();
        }
        /* Move to HSRUN Mode*/
        SMC->PMCTRL = SMC_PMCTRL_RUNM(0b11);
        /* Wait for Transition*/
        while(SMC->PMSTAT != 0x80);
        if (NULL != afterCb)
        {
            /* Execute callback after transition is done */
            afterCb();
        }
    }

}

void HSRUN_to_RUN (vfcn_callback beforeCb, vfcn_callback afterCb)
{
    /* Configure clocks before making the transisiton */
    Clock_run_configuration();

    /* Check if current mode is HSRUN mode */
    if(eHSRun == SMC->PMSTAT)
    {
        if (NULL != beforeCb)
        {
            /* Execute callback before doing the transition */
            beforeCb();
        }
        /* Move to RUN Mode*/
        SMC->PMCTRL = SMC_PMCTRL_RUNM(0b00);
        /* Wait for Transition*/
        while(SMC->PMSTAT != 0x01);
        if (NULL != afterCb)
        {
            /* Execute callback after transition is done */
            afterCb();
        }
    }
}



