/*
 * get_num.h -    Header file for get_num.c.
 *
 *  Created on: Oct 31, 2014
 *      Author: mam1
 */

/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2014.                   *
 *                                                                         *
 * This program is free software. You may use, modify, and redistribute it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation, either version 3 or (at your option)   *
 * any later version. This program is distributed without any warranty.    *
 * See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
 \*************************************************************************/

#ifndef GET_NUM_H_
#define GET_NUM_H_

#define GN_NONNEG       01      /* Value must be >= 0 */
#define GN_GT_0         02      /* Value must be > 0 */

/* By default, integers are decimal */
#define GN_ANY_BASE   0100      /* Can use any base - like strtol(3) */
#define GN_BASE_8     0200      /* Value is expressed in octal */
#define GN_BASE_16    0400      /* Value is expressed in hexadecimal */

long getLong(const char *arg, int flags, const char *name);

int getInt(const char *arg, int flags, const char *name);

#endif /* GET_NUM_H_ */
