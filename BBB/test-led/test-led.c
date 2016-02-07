
#include <stdio.h>
#include <unistd.h>   //sleep
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include <ctype.h>    //isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "shared.h"
#include "ipc.h"
#include "Pcon.h"
#include "shared.h"
#include "bitlit.h"
#include "PCF8563.h"
// #include "gpio.h"
// #include "led.h"
#include "schedule.h"
#include "BBBiolib.h"

unsigned int    gpio[4] = {_LED_1, _LED_2, _LED_3, _LED_4}; // map LEDs to gpio(s)

int main (void) {
  int del;

  printf("\n **** starting\n");

  printf("  iolib_init returned %i\n",iolib_init());
  printf("  iolib_setdirn returned", iolib_setdir(8, 8, BBBIO_DIR_OUT));

  int count = 0;
  del = 100; 
  while (count < 50)
  {
    count++ ;
    pin_high(8, 8);
    iolib_delay_ms(del);
    pin_low(8, 8);
    iolib_delay_ms(del);
  }

  iolib_free();
  printf("\n **** ending\n");
  return (0);
}

