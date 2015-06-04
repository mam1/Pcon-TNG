/*
 * Pcon.h
 *
 *  Created on: Nov 28, 2014
 *      Author: mam1
 */

#ifndef PCON_H_
#define PCON_H_

//#define _TRACE

#define reboot() __builtin_propeller_clkset(0x80)

/* shared codes, bone and prop */
#include "shared.h"
#include "typedefs.h"


/* functions */
//void disp_sys(void);

#endif /* PCON_H_ */
