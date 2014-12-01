/*
  Preliminary library code based on David Betz' code from
  compass code from code.google.com/p/propgcc.
*/

#include "../../../Simple_Libraries/Sensor/libcompass3d/compass3d.h"

#include "../../../Simple_Libraries/Protocol/libsimplei2c/simplei2c.h"
#include "../../../Simple_Libraries/Utility/libsimpletools/simpletools.h"

void compass_init(i2c *bus)
{
  /* set to continuous mode */
  unsigned char modeReg = 0x02;
  unsigned char contMode = 0x00;
  int n = i2c_out(bus, 0x3C, &modeReg, 1, &contMode, 1);
}

void compass_read(i2c *bus, int *px, int *py, int *pz)
{
  int16_t x16, y16, z16;
  uint8_t data[6];
  char datRegTo3 = 0x03;
  i2c_in(bus, 0x3D, &datRegTo3, 1, data, 6);

  x16 = (data[0] << 8) | data[1];
  z16 = (data[2] << 8) | data[3];
  y16 = (data[4] << 8) | data[5];

  *px = x16;
  *py = y16;
  *pz = z16;
}
