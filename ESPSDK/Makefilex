CC = xtensa-lx106-elf-gcc
CFLAGS = -I. -mlongcalls
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -Wl,--end-group -lgcc
LDFLAGS = -Teagle.app.v6.ld

t1-0x00000.bin: t1
	esptool.py elf2image $^

t1: t1.o

t1.o: t1.c

flash: t1-0x00000.bin
	esptool.py -p /dev/ttyUSB0 write_flash 0 t1-0x00000.bin 0x10000 t1-0x10000.bin

clean:
	rm -f t1 t1.o t1-0x00000.bin t1-0x10000.bin