/*
 * main implementation: use this 'C' sample to create your own application
 *
 */


#include "device_registers.h" /* include peripheral declarations S32K144 */
#include "lowPower.h"
#include "nvic.h"
#include "lptmr.h"
#include "wdog.h"
#include "config.h"
#include "delay.h"
#include "clock.h"

#include "spi.h"
#include "adc.h"

#define ENABLE_SPI          (1)
#define ENABLE_ADC          (1)

#define TOTAL_ADC_READS     (10)

volatile uint8_t goToSleep;

#if (DISABLE_DEBUGGER_PINS == 1)
void disablePins(void);
#endif

void lptmrCallback (void);

int main(void)
{
#if ENABLE_SPI
    uint32_t buffer[] = {0x11223344, 0x55667788};
#endif
#if ENABLE_ADC
    uint16_t adcReads[TOTAL_ADC_READS] __attribute__((unused)) = {0};
    const uint16_t adcChannels[TOTAL_ADC_READS] = {
            0b11101,    /* VREFH */
            0b11110,    /* VERFL */
            0b11011,    /* Bandgap */
            0,          /* ADCx_SE0 */
            1,          /* ADCx_SE1 */
            2,          /* ADCx_SE2 */
            3,          /* ADCx_SE3 */
            4,          /* ADCx_SE4 */
            5,          /* ADCx_SE5 */
            6           /* ADCx_SE6 */
    };
    uint32_t index = 0;
#endif
    /* Disable WDOG */
    WDOG_disable();

    /* Disable clock for other IPs to decrease power consumption */
    SIM->PLATCGC &= ~SIM_PLATCGC_CGCMPU_MASK;       /* Disable MPU */
    SIM->PLATCGC &= ~SIM_PLATCGC_CGCERM_MASK;       /* Disable ERM */
    SIM->PLATCGC &= ~SIM_PLATCGC_CGCEIM_MASK;       /* Disable EIM */
    SIM->PLATCGC &= ~SIM_PLATCGC_CGCMSCM_MASK;      /* Disable MSCM */
    SIM->PLATCGC &= ~SIM_PLATCGC_CGCDMA_MASK;       /* Disable DMA */

    /* Disable clock monitors on SCG module */
    disable_clock_monitors();
    /* Adjust SCG settings to meet maximum frequencies values */
    scg_run_configuration();

    /* Enable LPTMR to generate an interrupt every 50 ms */
    LPTMR_init(50, lptmrCallback);
    /* Enable LPTMR interrupt to wake the system on every interrupt */
    NVIC_EnableIRQ(LPTMR0_IRQn);

#if ENABLE_SPI
    /* Initialize LPSPI at 1MHz */
    LPSPI_init();
#endif
#if ENABLE_ADC
    /* Initialize ADC */
    ADC_init();
#endif

    goToSleep = 1;

#if (DISABLE_DEBUGGER_PINS == 1)
    disablePins();
#endif

    for(;;) {
        if (goToSleep == 1)
        {
#if ENABLE_SPI
            /* Need SPI initialization ? */
            LPSPI_sendData(buffer, sizeof(buffer) / sizeof(buffer[0]));
#endif
#if ENABLE_ADC
            /* Need ADC initialization ? */
            for (index = 0; index < TOTAL_ADC_READS; index++)
            {
                adcReads[index] = ADC_read(adcChannels[index]);
            }
            /* Evaluate adcReads */
#endif
            goToSleep = 0;
            Run_to_VLPS();
        }
    }
}


#if (DISABLE_DEBUGGER_PINS == 1)
void disablePins(void)
{
    PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK;    /* Enable clock for PORTA */
    PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;    /* Enable clock for PORTC */

    PORTA->PCR[10] = PORT_PCR_MUX(0);   /* Disable: PTA10 JTAG_TDO/noetm_TRACE_SWO */
    PORTA->PCR[4] = PORT_PCR_MUX(0);    /* Disable: PTA4 JTAG_TMS/SWD_DIO */
    PORTC->PCR[4] = PORT_PCR_MUX(0);    /* Disable: PTC4 JTAG_TCLK/SWD_CLK */

#if (RTC_USES_EXTERNAL_CLOCK == 0)
    PCC->PCCn[PCC_PORTA_INDEX] &= ~PCC_PCCn_CGC_MASK;    /* Enable clock for PORTA */
#endif
    PCC->PCCn[PCC_PORTC_INDEX] &= ~PCC_PCCn_CGC_MASK;    /* Enable clock for PORTC */
}
#endif

void lptmrCallback (void)
{
    goToSleep = 1;
}



