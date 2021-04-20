/*
 * lptmr.c
 *
 *  Created on: Apr 8, 2016
 *      Author: B46911
 */

#include "lptmr.h"
#include "error.h"

static vfcn_callback lptmr_callback = 0;

static void LPTMR_setPeriod (uint32_t ms);

void LPTMR_init(uint32_t ms, vfcn_callback callback)
{
    /* Enable LPTMR clock */
    PCC->PCCn[PCC_LPTMR0_INDEX] |= PCC_PCCn_CGC_MASK;

    LPTMR0->PSR = LPTMR_PSR_PCS(1) |     /* LPTMR clk src: LPO1K_CLK */
                    LPTMR_PSR_PBYP_MASK; /* Bypass Prescaler */
    lptmr_callback = callback;           /* save callback */
    LPTMR_setPeriod (ms);
    LPTMR0->CSR |= LPTMR_CSR_TIE_MASK;   /* Enable Interrupt request */
}

void LPTMR_start(void)
{
    /* Enable Timer */
    LPTMR0->CSR |= LPTMR_CSR_TEN_MASK;
}

void LPTMR_stop(void)
{
    /* Disable Timer */
    LPTMR0->CSR &= ~LPTMR_CSR_TEN_MASK;
}


void LPTMR0_IRQHandler (void)
{
    LPTMR0->CSR |= LPTMR_CSR_TCF_MASK;  /* Clear flag */
    if (lptmr_callback != 0)
    {
        lptmr_callback();
    }
}

static void LPTMR_setPeriod (uint32_t ms)
{
    uint32_t clockFreq = 0;
    uint32_t periodValue = 0;
    if (ms > 0)
    {
        switch((LPTMR0->PSR & LPTMR_PSR_PCS_MASK) >> LPTMR_PSR_PCS_SHIFT)
        {
        case 0:
            /* SIRC_DVI2 */
            if (0 != (SCG->SIRCDIV & SCG_SIRCDIV_SIRCDIV2_MASK))
            {
                clockFreq = 8000000 / (1 << (((SCG->SIRCDIV & SCG_SIRCDIV_SIRCDIV2_MASK) >> SCG_SIRCDIV_SIRCDIV2_SHIFT) - 1));
                /* Is prescaler not bypassed? */
                if (0 == (LPTMR0->PSR & LPTMR_PSR_PBYP_MASK))
                {
                    clockFreq /= (2 << ((LPTMR0->PSR & LPTMR_PSR_PRESCALE_MASK) >> LPTMR_PSR_PRESCALE_SHIFT));
                }
            }
            else
            {
                clockFreq = 0;
            }
            break;
        case 1:
            /* LPO1K */
            if (0 != (SIM->LPOCLKS & SIM_LPOCLKS_LPO1KCLKEN_MASK))
            {
                clockFreq = 1000;
                /* Is prescaler not bypassed? */
                if (0 == (LPTMR0->PSR & LPTMR_PSR_PBYP_MASK))
                {
                    clockFreq /= (2 << ((LPTMR0->PSR & LPTMR_PSR_PRESCALE_MASK) >> LPTMR_PSR_PRESCALE_SHIFT));
                }
            }
            else
            {
                clockFreq = 0;
            }
            break;
        case 2:
            break;
        case 3:
            break;
        default:
            break;
        }
        if (0 != clockFreq)
        {
            periodValue = (clockFreq * ms) / 1000;
            if (periodValue <= 0xFFFF)
            {
                LPTMR0->CMR = periodValue;
            }
            else
            {
                /* Error, LPTMR cannot be configured with current clock source value */
                error_trap();
            }
        }
        else
        {
            error_trap();
        }
    }
}



