/**
 * This is the main serial_inout program file.
 */
#include "simpletools.h"
#include "fdserial.h"

#define NONE 0
#define DEC 1
#define HEX 2
#define BIN 3
#define CHR 4

void serout(text_t *ser, int delay, char* string, int type, int val, int end);
void  serin(text_t *ser, int wait, void (*function)(), int type, void* value);

void fail() {
  print("FAIL\n");
}

int main(void)
{
  int inpin = 31;
  int outpin= 30;
  int baud  =115200;
  int value = 0x55;

  text_t *ser = fdserial_open(inpin, outpin, 0, baud);

  /* only used if we want to use print like in the fail function */
  if(inpin == 31 || outpin == 30) {
    extern terminal *dport_ptr;
    dport_ptr = ser;
  }

  pause(500);
  print("SEROUT/SERIN DEMO\n");

  while(1)
  {
    serout(ser, 100, "HELLO", 0, value, CR);
    serout(ser, 1, "DEC VAL ", DEC, value, CR);
    serout(ser, 1, "HEX VAL ", HEX, value, CR);
    serout(ser, 1, "BIN VAL ", BIN, value, CR);
    serout(ser, 1, "CHR VAL ", CHR, value, CR);

    print("Enter DEC: ");
    serin(ser, 5000, fail, DEC, &value);
    print("Got %d\n", value);

    print("Enter HEX: ");
    serin(ser, 5000, fail, HEX, &value);
    print("Got %x\n", value);

    print("Enter BIN: ");
    serin(ser, 5000, fail, BIN, &value);
    print("Got %b\n", value);

    print("Enter CHR: ");
    serin(ser, 5000, fail, CHR, &value);
    print("Got %c\n", value);
  }
  return 0;
}

void serout(text_t *ser, int delay, char* string, int type, int val, int end)
{
  int n = 0;
  int len = strlen(string);

  for(n = 0; n < len; n++) {
    writeChar(ser, string[n]);
    pause(delay);
  }

  if(DEC == type) {
    writeDec(ser, val);
    pause(delay);
    writeChar(ser, end);
  }
  else if(HEX == type) {
    writeHex(ser, val);
    pause(delay);
    writeChar(ser, end);
  }
  else if(BIN == type) {
    writeBin(ser, val);
    pause(delay);
    writeChar(ser, end);
  }
  else if(CHR == type) {
    writeChar(ser, val);
    pause(delay);
    writeChar(ser, end);
  }
  else {
    writeChar(ser, end);
  }
}


void  serin(text_t *ser, int wait, void (*function)(), int type, void* value)
{
  char buf[33]; // big enough for hex, dec, bin, or char

  while(--wait > -1) {
    if(fdserial_rxReady(ser))
      break;
    pause(1);
  }
  if(wait < 0) {
    writeLine(ser, "No input");
    function();
    return;
  }

  readStr(ser, buf, 33);

  if(DEC == type) {
    sscan(buf, "%d", value);
  }
  else if(HEX == type) {
    sscan(buf, "%x", value);
  }
  else if(BIN == type) {
    sscan(buf, "%b", value);
  }
  else if(CHR == type) {
    sscan(buf, "%c", value);
  }
}