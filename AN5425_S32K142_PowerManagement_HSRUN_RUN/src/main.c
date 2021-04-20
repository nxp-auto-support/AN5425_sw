/*
 * main implementation: use this 'C' sample to create your own application
 *
 */


#include "device_registers.h" /* include peripheral declarations S32K14x */

#include "LPUART.h"
#include "clocks.h"
#include "csec.h"
#include "PowerManagement.h"
#include "lptmr.h"
#include "pdb.h"
#include "nvic.h"
#include "error.h"
#include "port.h"
#include "ftm.h"

#define TOTAL_KEYS                  (17)
#define MESSAGE_SIZE                (4)

void disablePeripherals (void);
void enablePeripherals (void);

void timeoutCallback (void);
void PDB_callback (void);

uint32_t test_key[4] __attribute__((unused)) = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
uint32_t plain_text[MESSAGE_SIZE] __attribute__((aligned (16))) = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
uint32_t cipher_text[MESSAGE_SIZE] __attribute__((aligned (16))) = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

volatile uint8_t g_timeoutExpired = 0;


int main(void)
{
    /* Configure clocks for RUN mode */
    Clock_init();

    /* PORT initialization */
    PORT_init();

    /* Initialize console */
    init_LPUART1();				/* Initialize LPUART @ 115200*/

    printf("\r\nPower Management test\n\r");
    /* Initialize CSEc module */
    printf("Initializing CSEc module (Partition cmd)...");
    if (eCSEc_success != CSEc_init(TOTAL_KEYS, 0))
    {
        printf("[FAIL]\n\n");
    }
    else
    {
        printf("[OK]\n\n");
    }

    /* Load a RAM key to be used for encryption */
    printf("Load RAM key...");
    if (eCSEc_success != CSEc_load_plain_KEY(&test_key[0]))
    {
        printf("[FAIL]\n\n");
    }
    else
    {
        printf("[OK]\n\n");
    }

    /* Initialize PWM */
    FTM0_Init();

    /* Initialize timer to trigger power mode transitions */
    LPTMR_init(500, timeoutCallback);
    /* Enable timer interrupt */
    NVIC_EnableIRQ(LPTMR0_IRQn);

    /* Initialize timer to trigger power mode transitions */
    PDB0_init(PDB_callback);
    /* Enable timer interrupt */
    NVIC_EnableIRQ(PDB0_IRQn);
    /* Start PDB timer */
    PDB0_start();

    /* Switch to HSRUN mode */
    Run_to_HSRUN(disablePeripherals, enablePeripherals);

    g_timeoutExpired = 0;
    /* Start timer */
    LPTMR_start();

    while (1)
    {
        /* has timeout expired? then trigger a CSEc operation but before swith to RUN mode */
        if (g_timeoutExpired)
        {
            printf("Go to RUN to perform CSEc operations\n\r");
            /* Do power mode transition (RUN) to execute a CSEc operation */
            HSRUN_to_RUN(disablePeripherals, enablePeripherals);
            printf("[RUN_MODE] Encrypting data...");
            /* Encrypt data */
            if (eCSEc_success != CSEc_ENC_ECB(&cipher_text[0], &plain_text[0], RAM_KEY, sizeof(plain_text) / 16))
            {
                printf("[ECB_FAIL]\r\n\tError encrypting plain text\n\r");
                error_trap();
            }
            else
            {
                printf("[ECB_OK]\n\r");
            }
            printf("[RUN_MODE] Encrypting data...");
            /* Encrypt data */
            if (eCSEc_success != CSEc_ENC_ECB(&cipher_text[0], &plain_text[0], RAM_KEY, sizeof(plain_text) / 16))
            {
                printf("[ECB_FAIL]\r\n\tError encrypting plain text\n\r");
                error_trap();
            }
            else
            {
                printf("[ECB_OK]\n\r");
            }
            printf("Go to HSRUN to use maximum speed\n\r");
            /* Switch to HSRUN mode */
            Run_to_HSRUN(disablePeripherals, enablePeripherals);
            g_timeoutExpired = 0;
        }
    }
    return 0;
}

void timeoutCallback (void)
{
    g_timeoutExpired = 1;
}

void PDB_callback (void)
{
    PDB_TOGGLE_PIN;
}

void disablePeripherals (void)
{
    /* Stop PDB timer */
    PDB0_stop();
    /* Disable those peripherals using SYS_CLK / BUS_CLK for their reference */
    PWR_MODE_SET_PIN;
}

void enablePeripherals (void)
{
    /* Enable those peripherals using SYS_CLK / BUS_CLK for their reference */
    PWR_MODE_CLR_PIN;
    /* Start PDB timer */
    PDB0_start();
}

