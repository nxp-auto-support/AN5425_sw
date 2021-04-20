/*
 * config.h
 *
 *  Created on: Aug 29, 2017
 *      Author: B50982
 */

#ifndef CONFIG_H_
#define CONFIG_H_

/* SWD/JTAG pins */
#define DISABLE_DEBUGGER_PINS       (1)

/* SCG config */
#define CORE_FREQ_RUN_IN_MHZ        (4) /* 8MHz, 4MHz or 1MHz */
#define SCG_ENABLE_SIRC_IN_VLPS     (0)

/* LPTMR config */
#define LPTMR_USES_SIRC             (0)
#define LPTMR_ENABLE_PRESCALER      (0)
#define LPTMR_PRESCALER_VALUE       (6) /* 5: 64 as divider, 6: 128 as divider */

/* LPSPI */
#define LPSPI_PTR                   (LPSPI1)
#define LPSPI_CLK_INDEX             (PCC_LPSPI1_INDEX)
#define LPSPI_MODULE_IN_DEBUG       (0)
/* LPSPI1 pins */
#define LPSPI_PORT_NAME             (PORTB)
#define LPSPI_PORT_INDEX            (PCC_PORTB_INDEX)
#define LPSPI_SCK                   (14)
#define LPSPI_SIN                   (15)
#define LPSPI_SOUT                  (16)
#define LPSPI_PCS3                  (17)

/* ADC config */
#define ADC_CLK_INDEX               (PCC_ADC0_INDEX)
#define ADC_PTR                     (ADC0)

/* LED config */
#define PORT_PCC_INDEX              (PCC_PORTD_INDEX)
#define PORT_PTR                    (PORTD)
#define GPIO_PTR                    (PTD)
#define GPIO_PIN                    (15)

#endif /* CONFIG_H_ */
