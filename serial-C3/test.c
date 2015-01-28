#include <propeller.h>
#include "test.h"

int main(void)
{
  int32_t	Val;
  waitcnt(CLKFREQ+CNT);
  fdsSpin_Start(31, 30, 0, 115200);
  fdsSpin_Str("Hello World\r");
  while (1) {
    Val = fdsSpin_Rx();
    fdsSpin_Tx(Val);
  }
  return 0;
}

