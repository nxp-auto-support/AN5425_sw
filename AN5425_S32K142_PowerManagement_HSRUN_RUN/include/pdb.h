/*
 * pdb.h
 *
 *  Created on: Apr 8, 2016
 *      Author: B46911
 */

#ifndef PDB_H_
#define PDB_H_

#include "device_registers.h" /* include peripheral declarations */

typedef void (*vfcn_callback)(void);

void PDB0_init(vfcn_callback cb);

void PDB0_start(void);

void PDB0_stop(void);


#endif /* PDB_H_ */
