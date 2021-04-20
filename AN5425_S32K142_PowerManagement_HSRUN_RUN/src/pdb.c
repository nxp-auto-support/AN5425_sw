/*
 * pdb.c
 *
 *  Created on: Apr 8, 2016
 *      Author: B46911
 */

#include "pdb.h"
#include "error.h"

static vfcn_callback pdb_callback = 0;

void PDB0_init(vfcn_callback cb)
{
    /* ----- PDB configuration ----- */
        PCC->PCCn[PCC_PDB0_INDEX] |= PCC_PCCn_CGC_MASK;     /* Enable PDB clock */

        /* PDB timeout is 1 / (112MHz / (8 * 40)) * 360 ~ 1 ms in HSRUN, 1 / (56MHz / (8 * 40)) * 360 ~ 2 ms / */
        PDB0->MOD = PDB_MOD_MOD(360);      /* Period is set to 22 ms in HSRUN, 45 ms in RUN */
        PDB0->IDLY = 360;                  /* Interrupt is set on 22 ms in HSRUN, 45 ms in RUN */

        /* Save callback */
        pdb_callback = cb;

        PDB0->SC = PDB_SC_PRESCALER(3) |    /* PDB frequency is: PDB clock (System Clock) / ((2^PRESCALER) * MULT) */
                    PDB_SC_MULT(3) |        /* MULT value set to 40 */
                    PDB_SC_PDBIE_MASK |     /* Enable interrupt for PDB */
                    PDB_SC_LDMOD(0)|        /* Load mode 0: The internal registers are loaded with the values from their buffers  immediately after 1 is written to SC[LDOK]. */
                    PDB_SC_TRGSEL(0b1111)|  /*Software trigger*/
                    PDB_SC_CONT(1);         /*Continous mode*/

        //PDB0->SC |= PDB_SC_LDOK_MASK;       /* Load values for MOD, IDLY, CHnDLYm, and POyDLY registers */
}

void PDB0_start(void)
{
    /* Enable PDB */
    PDB0->SC |= PDB_SC_PDBEN_MASK |
            PDB_SC_LDOK_MASK;
    /* Send a SW trigger */
    PDB0->SC |= PDB_SC_SWTRIG_MASK;
}

void PDB0_stop(void)
{
    /* Disable PDB */
    PDB0->SC &= ~PDB_SC_PDBEN_MASK;
}

void PDB0_IRQHandler (void)
{
    /* Check for IDLY interrupt */
    if ((PDB0->SC & PDB_SC_PDBIF_MASK) != 0)
    {
        /* Clear PDBIF flag */
        PDB0->SC &= ~(PDB_SC_PDBIF_MASK);
        /* Callback execution */
        if (0 != pdb_callback)
        {
            pdb_callback();
        }

    }
}



