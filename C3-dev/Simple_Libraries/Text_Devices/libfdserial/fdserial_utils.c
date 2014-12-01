/**
 * @file fdserial.c
 * Full Duplex Serial adapter module.
 *
 * Copyright (c) 2008-2013, Steve Denson
 * See end of file for terms of use.
 */
#include <propeller.h>

#include "../../../Simple_Libraries/Text_Devices/libfdserial/fdserial.h"

/*
 * rxflush empties the receive queue 
 */
void fdserial_rxFlush(fdserial *term)
{
  while(fdserial_rxCheck(term) >= 0)
      ; // clear out queue by receiving all available 
}

/*
 * Check if a byte is available in the buffer.
 * Function does not block.
 * @returns non-zero if a byte is available.
 */
int fdserial_rxReady(fdserial *term)
{
  volatile fdserial_st* fdp = (fdserial_st*) term->devst;
  return (fdp->rx_tail != fdp->rx_head);
}

/*
 * Get a byte from the receive queue if available within timeout period.
 * Function blocks if no recieve for ms timeout.
 * @param ms is number of milliseconds to wait for a char
 * @returns receive byte 0 to 0xff or -1 if none available 
 */
int fdserial_rxTime(fdserial *term, int ms)
{
  int rc = -1;
  int t1 = 0;
  int t0 = CNT;
  do {
      rc = fdserial_rxCheck(term);
      t1 = CNT;
      if((t1 - t0)/(CLKFREQ/1000) > ms)
          break;
  } while(rc < 0);
  return rc;
}

void fdserial_txFlush(fdserial *term)
{
  while(!fdserial_txEmpty(term));
}


/*
+--------------------------------------------------------------------
| TERMS OF USE: MIT License
+--------------------------------------------------------------------
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
+--------------------------------------------------------------------
*/

