/*
 * main implementation: use this 'C' sample to create your own application
 *
 */

#include "device_registers.h" /* include peripheral declarations S32K116 */
#include "lowPower.h"
#include "lpuart.h"
#include "error.h"
#include "delay.h"

static volatile uint8_t g_wakeUp = 0;

static void lpuartComplete (void);

#if (1 == DISABLE_DEBUGGER_PINS)
static void disableSWDpins(void);
#endif

static uint8_t txBuffer[LPUART_BUFFER_SIZE] = {0};

int main(void)
{

    /* All data in buffer is zero except last element */
    txBuffer[LPUART_BUFFER_SIZE - 1] = 0xAA;

    SIM->PLATCGC &= ~SIM_PLATCGC_CGCMPU_MASK;   /* Disable MPU */
    SIM->PLATCGC &= ~SIM_PLATCGC_CGCERM_MASK;   /* Disable ERM */
    SIM->PLATCGC &= ~SIM_PLATCGC_CGCEIM_MASK;   /* Disable EIM */
    SIM->PLATCGC &= ~SIM_PLATCGC_CGCMSCM_MASK;  /* Disable MSCM */

     /* Switch to VLPR mode */
    Run_to_VLPR();

    /* Go to sleep */
    g_wakeUp = 0;

    /* Initialize LPUART */
    LPUART_dmaInit();

    /* prepare DMA */
    LPUART_prepareDMA(&txBuffer[0], sizeof(txBuffer), lpuartComplete);

#if (1 == DISABLE_DEBUGGER_PINS)
    /* Disable debug pins */
    disableSWDpins();
#endif

    /* enable DMA request */
    LPUART_sendData();

    /* Transition to VLPS is done */
    VLPR_to_VLPS();

    for(;;)
    {
        if (g_wakeUp)
        {
            g_wakeUp = 0;

            /* If another transaction is needed (different buffer, different size, etc),
             * DMA configuration is required, in case the same buffer/length is intended to be sent,
             * then prepare DMA function can be omitted as only the DMA trigger is needed */
            LPUART_prepareDMA(&txBuffer[0], sizeof(txBuffer), lpuartComplete);
            /* Enable request for DMA channel connected to LPUART */
            LPUART_sendData();
            /* Transition to VLPS is done */
            VLPR_to_VLPS();
        }
    }
    return 0;
}

#if (1 == DISABLE_DEBUGGER_PINS)
static void disableSWDpins(void)
{
    PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK;    /* Enable clock for PORTA */
    PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;    /* Enable clock for PORTC */

    PORTA->PCR[4] = PORT_PCR_MUX(0);    /* Disable: PTA4 JTAG_TMS/SWD_DIO */
    PORTC->PCR[4] = PORT_PCR_MUX(0);    /* Disable: PTC4 JTAG_TCLK/SWD_CLK */

    PCC->PCCn[PCC_PORTC_INDEX] &= ~PCC_PCCn_CGC_MASK;    /* Enable clock for PORTC */
}
#endif

static void lpuartComplete (void)
{
    g_wakeUp = 1;
}

