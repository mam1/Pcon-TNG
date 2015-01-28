

CFLAGS = -Os -m32bit-doubles -Wall -std=c99 
NM = propeller-elf-nm
MODEL = lmm
BOARD =  QUICKSTART

# objects for this program
NAME = test
OBJS = test.o fds.o

#FdSerial.o serial_io.o

all: $(NAME).elf

include common.mk

#
# do a partial link of all driver code
#

# # Note that calling our output file *_1.cog ensures that it
# is placed in the .coguser1 section by the main program's
# linker script; similarly *_2.cog would be placed in .coguser2,
# and so on

# the raw C code for the cog
fds_ht.o: full_duplex_serial_ht_cog.c
	$(CC) -r -Os -mcog -o $@ $<

#
# We have to avoid conflicts between symbols in the main C program and
# symbols in the local cog C programs. We do this by using objcopy to
# turn all the global symbols in the cog .text segment into local symbols
# (that's what --localize-text does).
#

%.cog: %.o
	$(OBJCOPY) --rename-section .text=$@ --localize-text $^ $@

