/*
 * flash.c
 *
 *  Created on: Jan 23, 2017
 *      Author: B50982
 */

#include "flash.h"

#define NULL 0L

__attribute__ ((section(".code_ram"))) /* Flash command is launched from RAM */
eCommandStatus flash_launchCommand(void)
{
    /* Launch command */
    FTFC->FSTAT |= FTFC_FSTAT_CCIF_MASK;

    /* Wait for previous command to complete */
    while ((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) == 0);

    return flash_check_errors();
}

eCommandStatus flash_command(uint8_t const *param, uint8_t size)
{
    eCommandStatus ret = eFlash_CommandError;
    uint8_t index = 0;

    if ((NULL != param) && (0 < size))
    {
        ret = eFlash_NoError;
        /* Wait for previous command to complete */
        while ((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) == 0);
        /* Check for errors */
        if (eFlash_NoError != flash_check_errors())
        {
            /* Clear all error flags */
            flash_clear_errors(eFlash_ReadCollisionError |
                               eFlash_AccessError |
                               eFlash_ProtectionViolation);
        }
        /* As FCCOB registers does not match with every index, implement and adjust to
         * match the proper FCCOB register, they are located as follows:
         *  FCCOB3
         *  FCCOB2
         *  FCCOB1
         *  FCCOB0
         *  FCCOB7
         *  FCCOB6
         *  FCCOB5
         *  FCCOB4
         *  FCCOBB
         *  FCCOBA
         *  FCCOB9
         *  FCCOB8
         *  */
        for (index = 0; index < size; index++)
        {
            switch (index / 4)
            {
            case (0):
                FTFC->FCCOB[3 - index] = param[index];
                break;
            case (1):
                    /* Adjust to match register index for 7 to 4 */
                FTFC->FCCOB[11 - index] = param[index];
                break;
            case (2):
                    /* Adjust to match register index for 8 to B */
                FTFC->FCCOB[19 - index] = param[index];
                break;
            default:
                break;
            }
        }
        ret = flash_launchCommand();

    }
    return ret;
}

eCommandStatus flash_check_errors (void)
{
    eCommandStatus ret = eFlash_NoError;

    /* Check for errors */
    if (FTFC->FSTAT & FTFC_FSTAT_RDCOLERR_MASK)
    {
        FTFC->FSTAT |= FTFC_FSTAT_RDCOLERR_MASK;
        ret |= eFlash_ReadCollisionError;
    }

    if (FTFC->FSTAT & FTFC_FSTAT_ACCERR_MASK)
    {
        FTFC->FSTAT |= FTFC_FSTAT_ACCERR_MASK;
        ret |= eFlash_AccessError;
    }

    if (FTFC->FSTAT & FTFC_FSTAT_FPVIOL_MASK)
    {
        /* Clear all error flags */
        FTFC->FSTAT |= FTFC_FSTAT_FPVIOL_MASK;
        ret |= eFlash_ProtectionViolation;
    }

    if (FTFC->FSTAT & FTFC_FSTAT_MGSTAT0_MASK)
    {
        ret |= eFlash_CommandError;
    }
    return ret;
}

void flash_clear_errors (eCommandStatus flags)
{
    /* Check for errors */
    if (flags & eFlash_ReadCollisionError)
    {
        FTFC->FSTAT |= FTFC_FSTAT_RDCOLERR_MASK;
    }

    if (flags & eFlash_AccessError)
    {
        FTFC->FSTAT |= FTFC_FSTAT_ACCERR_MASK;
    }

    if (flags & eFlash_ProtectionViolation)
    {
        /* Clear all error flags */
        FTFC->FSTAT |= FTFC_FSTAT_FPVIOL_MASK;
    }
}

