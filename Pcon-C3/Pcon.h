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
//#include "../Pcon-share/schedule.h"
#include "typedefs.h"

// #include "../Pcon-share/shared.h"

/* functions */
//void disp_sys(void);
// void term(FdSerial_t *);

#endif /* PCON_H_ */
