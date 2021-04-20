/*
 * clocks.c
 *
 *  Created on: Jan 9, 2018
 *      Author: B50982
 */

#include "clocks.h"

#define SOSC_VALUE                  (8000000UL)
#define SIRC_VALUE                  (8000000UL)
#define FIRC_VALUE                  (48000000UL)

/* Macro to get/set DIV1 and DIV2 values for asynchronous sources */
#define GET_ASYNC_CLK_DIV2(reg)     ((reg & (0x700)) >> 8)
#define GET_ASYNC_CLK_DIV1(reg)     ((reg & (0x7)) >> 0)

/* Macro to get PREDIV and MULT values for VCO calculation */
#define SPLL_PREDIV_VAL()           ((SCG->SPLLCFG & SCG_SPLLCFG_PREDIV_MASK) >> SCG_SPLLCFG_PREDIV_SHIFT)
#define SPLL_MULT_VAL()             ((SCG->SPLLCFG & SCG_SPLLCFG_MULT_MASK) >> SCG_SPLLCFG_MULT_SHIFT)

/* Macro to set maximum clock values for Core, Bus and Flash in HSRUN */
#define MAX_CORE_CLK_HSRUN          (112000000)
#define MIN_CORE_CLK_HSRUN          (80000000)
#define MAX_BUS_CLK_HSRUN           (MAX_CORE_CLK_HSRUN / 2)
#define MAX_FLASH_CLK_HSRUN         (MAX_CORE_CLK_HSRUN / 4)
#define MAX_SPLLDIV1_CLK_HSRUN      (MAX_CORE_CLK_HSRUN)
#define MAX_SPLLDIV2_CLK_HSRUN      (MAX_CORE_CLK_HSRUN / 2)
#define MAX_FIRCDIV1_CLK_HSRUN      (FIRC_VALUE)
#define MAX_FIRCDIV2_CLK_HSRUN      (FIRC_VALUE)

/* Macro to set maximum clock values for Core, Bus and Flash in RUN */
#define MAX_CORE_CLK_RUN            (80000000)
#define MAX_BUS_CLK_RUN             (MAX_CORE_CLK_RUN / 2)
#define MAX_FLASH_CLK_RUN           (MAX_CORE_CLK_RUN / 3)
#define MAX_SPLLDIV1_CLK_RUN        (MAX_CORE_CLK_RUN)
#define MAX_SPLLDIV2_CLK_RUN        (MAX_CORE_CLK_RUN / 2)
#define MAX_FIRCDIV1_CLK_RUN        (FIRC_VALUE)
#define MAX_FIRCDIV2_CLK_RUN        (FIRC_VALUE)


/* Possible clock sources */
typedef enum {
    eCoreClock = 0,
    eBusClock,
    eFlashClock,
    eSIRClk,
    eFIRClk,
    eSysOsc,
    eSysPLL,
    eSysPLL_div1,
    eSysPLL_div2,
    eFIRC_div1,
    eFIRC_div2,
    eSIRC_div1,
    eSIRC_div2,
    eSysOsc_div1,
    eSysOsc_div2
}eClock;

typedef enum {
    SOSC = 1,
    SIRC,
    FIRC,
    SPLL = 6
}eSystemClockSource;

static void OSC_init (void);
static void PLL_init (void);
static void FIRC_init (void);
static void Set_RUN_dividers (void);
static void Set_HSRUN_dividers (void);

static uint32_t get_clock_freq (eClock src);
static uint32_t SCG_get_firc_freq(void);
static uint32_t SCG_get_sirc_freq(void);
static uint32_t SCG_get_sosc_freq(void);
static uint32_t SCG_get_spll_freq(void);

static uint32_t SCG_get_core_freq(void);
static uint32_t SCG_get_bus_freq(void);
static uint32_t SCG_get_flash_freq(void);

static uint32_t SCG_get_hsrun_core_freq(void);
static uint32_t SCG_get_hsrun_bus_freq(void);
static uint32_t SCG_get_hsrun_flash_freq(void);

static uint32_t SCG_get_run_core_freq(void);
static uint32_t SCG_get_run_bus_freq(void);
static uint32_t SCG_get_run_flash_freq(void);

void Clock_init (void)
{
    /* Initialize OSC */
    OSC_init();

    /* Initialize PLL */
    PLL_init();

    /* Initialize FIRC */
    FIRC_init();

    /* Set core, bus and flash dividers for RUN mode */
    Set_RUN_dividers();
}

uint8_t Clock_run_configuration (void)
{
    uint8_t ret = 0;
    /* Set core, bus and flash dividers for RUN mode */
    Set_RUN_dividers();

    /* Check clocks limits in RUN mode */
    if (MAX_CORE_CLK_RUN < SCG_get_run_core_freq())
    {
        /* Core speed is higher than maximum value */
        ret = ~0;
    }
    else if (MAX_BUS_CLK_RUN < SCG_get_run_bus_freq())
    {
        /* Bus speed is higher than maximum value */
        ret = ~0;
    }
    else if (MAX_FLASH_CLK_RUN < SCG_get_run_flash_freq())
    {
        /* Flash speed is higher than maximum value */
        ret = ~0;
    }
    else if (MAX_SPLLDIV1_CLK_RUN < get_clock_freq(eSysPLL_div1))
    {
        /* SPLL_DIV1 speed is higher than maximum value */
        ret = ~0;
    }
    else if (MAX_SPLLDIV2_CLK_RUN < get_clock_freq(eSysPLL_div2))
    {
        /* SPLL_DIV2 speed is higher than maximum value */
        ret = ~0;
    }
    else if (MAX_FIRCDIV1_CLK_RUN < get_clock_freq(eFIRC_div1))
    {
        /* FIRC_DIV1 speed is higher than maximum value */
        ret = ~0;
    }
    else if (MAX_FIRCDIV2_CLK_RUN < get_clock_freq(eFIRC_div2))
    {
        /* FIRC_DIV2 speed is higher than maximum value */
        ret = ~0;
    }
    return ret;
}

uint8_t Clock_hsrun_configuration (void)
{
    uint8_t ret = 0;

    /* Set core, bus and flash dividers for RUN mode */
    Set_HSRUN_dividers();

    /* Check core clock in HSRUN mode */
    if ((MAX_CORE_CLK_HSRUN < SCG_get_hsrun_core_freq()) && (MIN_CORE_CLK_HSRUN > SCG_get_hsrun_core_freq()))
    {
        /* Core speed is higher than maximum value */
        ret = ~0;
    }
    else if (MAX_BUS_CLK_HSRUN < SCG_get_hsrun_bus_freq())
    {
        /* Bus speed is higher than maximum value */
        ret = ~0;
    }
    else if (MAX_FLASH_CLK_HSRUN < SCG_get_hsrun_flash_freq())
    {
        /* Flash speed is higher than maximum value */
        ret = ~0;
    }
    else if (MAX_SPLLDIV1_CLK_HSRUN < get_clock_freq(eSysPLL_div1))
    {
        /* SPLL_DIV1 speed is higher than maximum value */
        ret = ~0;
    }
    else if (MAX_SPLLDIV2_CLK_HSRUN < get_clock_freq(eSysPLL_div2))
    {
        /* SPLL_DIV2 speed is higher than maximum value */
        ret = ~0;
    }
    else if (MAX_FIRCDIV1_CLK_HSRUN < get_clock_freq(eFIRC_div1))
    {
        /* FIRC_DIV1 speed is higher than maximum value */
        ret = ~0;
    }
    else if (MAX_FIRCDIV2_CLK_HSRUN < get_clock_freq(eFIRC_div2))
    {
        /* FIRC_DIV2 speed is higher than maximum value */
        ret = ~0;
    }
    return ret;
}

static void OSC_init (void)
{
    SCG->SOSCCFG = SCG_SOSCCFG_EREFS_MASK |     /* Selects the output of the OSC logic (crystal is used) */
            SCG_SOSCCFG_RANGE(0b11);    /* Range is very high frequency crystal (8MHz) */

    SCG->SOSCCSR &= ~SCG_SOSCCSR_LK_MASK;       /* Unlock the register */

    SCG->SOSCCSR |= SCG_SOSCCSR_SOSCEN_MASK;    /* Enable System Oscillator */

    while (0 == (SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK)) { };  /* Wait until SOSC is enabled */

    SCG->SOSCDIV = SCG_SOSCDIV_SOSCDIV1(0) |    /* Disable SOSC_DIV1 and SOSC_DIV2 */
            SCG_SOSCDIV_SOSCDIV2(0);
}

static void PLL_init (void)
{
    /* PLL output = ((OSC_value) / (prediv + 1) * (mult + 16) ) / 2 */
    /* PLL output = ((8MHz) / (0 + 1) * (12 + 16) ) / 2 = 112 MHz */
    /* Configure SPLL dividers to get 112 MHz output */
    SCG->SPLLCFG = SCG_SPLLCFG_MULT(12) |
            SCG_SPLLCFG_PREDIV(0);
    /* Enable System PLL */
    SCG->SPLLCSR |= SCG_SPLLCSR_SPLLEN_MASK;
    /* Wait until SPLL is enabled */
    while (0 == (SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK)) { };

    SCG->SPLLDIV = SCG_SPLLDIV_SPLLDIV1(4) |    /* SPLL_DIV1 set to 14 MHz (112MHz / 8) */
            SCG_SPLLDIV_SPLLDIV2(3);            /* SPLL_DIV2 set to 28 MHz (112MHz / 4) */
}

static void FIRC_init (void)
{
    /* Is FIRC not valid? */
    if (0 == (SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK))
    {
        /* Enable FIRC and clear errors (if present) */
        SCG->FIRCCSR = SCG_FIRCCSR_FIRCERR_MASK |
                SCG_FIRCCSR_FIRCEN_MASK;
        /* Wait until FIRC is enabled */
        while (0 == (SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK)) { };
    }
    SCG->FIRCDIV = SCG_FIRCDIV_FIRCDIV1(2) |    /* FIRC_DIV1 set to 24 MHz (48 MHz / 2) */
            SCG_FIRCDIV_FIRCDIV2(2);    /* FIRC_DIV2 set to 24 MHz (48 MHz / 2) */
}

static void Set_RUN_dividers (void)
{
    /* Set DIVCORE value for RUN */
    SCG->RCCR = SCG_CSR_SCS(6) |        /* Select PLL */
            SCG_CSR_DIVCORE(1) |    /* PLL output divided by 2 to generate Core clock 56 MHz (112 MHz / 2) */
            SCG_CSR_DIVBUS(1) |     /* Core clock divided by 2 to generate Bus clock 28 MHz (56 MHz / 2) */
            SCG_CSR_DIVSLOW(3);     /* Core clock divided by 4 to generate Flash clock 14 MHz (56 MHz / 2) */

    /* Wait until PLL is selected to generate core, bus and flash clocks */
    while (6 != ((SCG->CSR & SCG_CSR_SCS_MASK) >> SCG_CSR_SCS_SHIFT)) {}

}

static void Set_HSRUN_dividers (void)
{
    /* Set DIVCORE value for HSRUN */
    SCG->HCCR = SCG_CSR_SCS(6) |        /* Select PLL */
            SCG_CSR_DIVCORE(0) |    /* PLL output divided by 1 to generate Core clock 112 MHz (112 MHz / 1) */
            SCG_CSR_DIVBUS(1) |     /* Core clock divided by 2 to generate Bus clock 56 MHz (112 MHz / 2) */
            SCG_CSR_DIVSLOW(3);     /* Core clock divided by 4 to generate Flash clock 28 MHz (112 MHz / 4) */

    /* Wait until PLL is selected to generate core, bus and flash clocks */
    while (6 != ((SCG->CSR & SCG_CSR_SCS_MASK) >> SCG_CSR_SCS_SHIFT)) {}
}

static uint32_t get_clock_freq (eClock src)
{
    uint32_t freq = 0, div = 0;
    switch (src)
    {
    case eCoreClock:
        freq = SCG_get_core_freq();
        break;
    case eBusClock:
        freq = SCG_get_bus_freq();
        break;
    case eFlashClock:
        freq = SCG_get_flash_freq();
        break;
    case eSIRClk:
        freq = SCG_get_sirc_freq();
        break;
    case eFIRClk:
        freq = SCG_get_firc_freq();
        break;
    case eSysOsc:
        freq = SCG_get_sosc_freq();
        break;
    case eSysPLL:
        freq = SCG_get_spll_freq();
        break;
    case eSysPLL_div1:
    case eSysPLL_div2:
    {
        /* Get frequency for SPLL */
        freq = SCG_get_spll_freq();
        /* Get divider value for SPLLDIV register */
        div = (src == eSysPLL_div1) ? GET_ASYNC_CLK_DIV1(SCG->SPLLDIV) :
                GET_ASYNC_CLK_DIV2(SCG->SPLLDIV);
        /* Output disabled ? */
        if (0 == div)
        {
            freq = 0;
        }
        else
        {
            /* Calculate divider factor for div field value */
            div = (1 << (div - 1));
            /* Divide source frequency */
            freq /= div;
        }
    }
    break;
    case eFIRC_div1:
    case eFIRC_div2:
    {
        /* Get frequency for FIRC */
        freq = SCG_get_firc_freq();
        /* Get divider value for FIRCDIV register */
        div = (src == eFIRC_div1) ? GET_ASYNC_CLK_DIV1(SCG->FIRCDIV) :
                GET_ASYNC_CLK_DIV2(SCG->FIRCDIV);
        /* Output disabled ? */
        if (0 == div)
        {
            freq = 0;
        }
        else
        {
            /* Calculate divider factor for div field value */
            div = (1 << (div - 1));
            /* Divide source frequency */
            freq /= div;
        }
    }
    break;
    case eSIRC_div1:
    case eSIRC_div2:
    {
        /* Get frequency for SIRC */
        freq = SCG_get_sirc_freq();
        /* Get divider value for SIRCDIV register */
        div = (src == eSIRC_div1) ? GET_ASYNC_CLK_DIV1(SCG->SIRCDIV) :
                GET_ASYNC_CLK_DIV2(SCG->SIRCDIV);
        /* Output disabled ? */
        if (0 == div)
        {
            freq = 0;
        }
        else
        {
            /* Calculate divider factor for div field value */
            div = (1 << (div - 1));
            /* Divide source frequency */
            freq /= div;
        }
    }
    break;
    case eSysOsc_div1:
    case eSysOsc_div2:
    {
        /* Get frequency for SOSC */
        freq = SCG_get_sosc_freq();
        /* Get divider value for SOSCDIV register */
        div = (src == eSysOsc_div1) ? GET_ASYNC_CLK_DIV1(SCG->SOSCDIV) :
                GET_ASYNC_CLK_DIV2(SCG->SOSCDIV);
        /* Output disabled ? */
        if (0 == div)
        {
            freq = 0;
        }
        else
        {
            /* Calculate divider factor for div field value */
            div = (1 << (div - 1));
            /* Divide source frequency */
            freq /= div;
        }
    }
    break;
    default:
        break;
    }
    return freq;
}

static uint32_t SCG_get_firc_freq(void)
{
    uint32_t freq = 0;
    if (((SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK) >> SCG_FIRCCSR_FIRCVLD_SHIFT) == 1)
    {
        freq = FIRC_VALUE;
    }
    return freq;
}

static uint32_t SCG_get_sirc_freq(void)
{
    uint32_t freq = 0;
    if (((SCG->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK) >> SCG_SIRCCSR_SIRCVLD_SHIFT) == 1)
    {
        freq = SIRC_VALUE;
    }
    return freq;
}

static uint32_t SCG_get_sosc_freq(void)
{
    uint32_t freq = 0;
    if (((SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK) >> SCG_SOSCCSR_SOSCVLD_SHIFT) == 1)
    {
        freq = SOSC_VALUE;
    }
    return freq;
}

static uint32_t SCG_get_spll_freq(void)
{
    uint32_t freq = 0;
    if (((SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK) >> SCG_SPLLCSR_SPLLVLD_SHIFT) == 1)
    {
        /* VCO = SOSC_CLK/(PREDIV + 1) X (MULT + 16) */
        /* SPLL_CLK = (VCO_CLK) / 2 */
        freq = ((SCG_get_sosc_freq() / (SPLL_PREDIV_VAL() + 1)) * (SPLL_MULT_VAL() + 16)) >> 1;
    }
    return freq;
}

static uint32_t SCG_get_core_freq(void)
{
    uint32_t freq = 0, div = (((SCG->CSR & SCG_CSR_DIVCORE_MASK) >> SCG_CSR_DIVCORE_SHIFT) + 1);
    switch ((SCG->CSR & SCG_CSR_SCS_MASK) >> SCG_CSR_SCS_SHIFT)
    {
    case SOSC:
        freq = SCG_get_sosc_freq() / div;
        break;
    case SIRC:
        freq = SCG_get_sirc_freq() / div;
        break;
    case FIRC:
        freq = SCG_get_firc_freq() / div;
        break;
    case SPLL:
        freq = SCG_get_spll_freq() / div;
        break;
    default:
        break;
    }
    return freq;
}

static uint32_t SCG_get_bus_freq(void)
{
    uint32_t div = (((SCG->CSR & SCG_CSR_DIVBUS_MASK) >> SCG_CSR_DIVBUS_SHIFT) + 1);
    return SCG_get_core_freq() / div;
}

static uint32_t SCG_get_flash_freq(void)
{
    uint32_t div = (((SCG->CSR & SCG_CSR_DIVSLOW_MASK) >> SCG_CSR_DIVSLOW_SHIFT) + 1);
    return SCG_get_core_freq() / div;
}

static uint32_t SCG_get_hsrun_core_freq(void)
{
    uint32_t freq = 0, div = (((SCG->HCCR & SCG_HCCR_DIVCORE_MASK) >> SCG_HCCR_DIVCORE_SHIFT) + 1);
    switch ((SCG->HCCR & SCG_HCCR_SCS_MASK) >> SCG_HCCR_SCS_SHIFT)
    {
    case FIRC:
        freq = SCG_get_firc_freq() / div;
        break;
    case SPLL:
        freq = SCG_get_spll_freq() / div;
        break;
    default:
        break;
    }
    return freq;
}

static uint32_t SCG_get_hsrun_bus_freq(void)
{
    uint32_t div = (((SCG->HCCR & SCG_HCCR_DIVBUS_MASK) >> SCG_HCCR_DIVBUS_SHIFT) + 1);
    return SCG_get_hsrun_core_freq() / div;
}

static uint32_t SCG_get_hsrun_flash_freq(void)
{
    uint32_t div = (((SCG->HCCR & SCG_HCCR_DIVSLOW_MASK) >> SCG_HCCR_DIVSLOW_SHIFT) + 1);
    return SCG_get_hsrun_core_freq() / div;
}

static uint32_t SCG_get_run_core_freq(void)
{
    uint32_t freq = 0, div = (((SCG->RCCR & SCG_RCCR_DIVCORE_MASK) >> SCG_RCCR_DIVCORE_SHIFT) + 1);
    switch ((SCG->RCCR & SCG_RCCR_SCS_MASK) >> SCG_RCCR_SCS_SHIFT)
    {
    case SOSC:
        freq = SCG_get_sosc_freq() / div;
        break;
    case SIRC:
        freq = SCG_get_sirc_freq() / div;
        break;
    case FIRC:
        freq = SCG_get_firc_freq() / div;
        break;
    case SPLL:
        freq = SCG_get_spll_freq() / div;
        break;
    default:
        break;
    }
    return freq;
}

static uint32_t SCG_get_run_bus_freq(void)
{
    uint32_t div = (((SCG->RCCR & SCG_RCCR_DIVBUS_MASK) >> SCG_RCCR_DIVBUS_SHIFT) + 1);
    return SCG_get_run_core_freq() / div;
}

static uint32_t SCG_get_run_flash_freq(void)
{
    uint32_t div = (((SCG->RCCR & SCG_RCCR_DIVSLOW_MASK) >> SCG_RCCR_DIVSLOW_SHIFT) + 1);
    return SCG_get_run_core_freq() / div;
}


