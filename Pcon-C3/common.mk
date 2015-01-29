# #########################################################
# This makefile fragment builds LMM/XMM/XMMC demo programs
#   
# To use it, define:
#  PROPLIB to be the path to this directory
#  NAME to be the name of project
#       - this is used to create the final program $(NAME).elf
#  OBJS to be the object files needed for the project
#  MODEL to lmm, xmm, or xmmc
#  CFLAGS to be desired CFLAGS
#
#  Then set up a default "all" target (normally this will be
#    all: $(NAME).elf
#  and finally
#    include $(PROPLIB)/demo.mk
#
# Copyright (c) 2011 Parallax Inc.
# All rights MIT licensed
# #########################################################


# get the development machine OS
UNAME_S := $(shell uname -s)
UNAME_P := $(shell uname -p)

# set include directories
ifeq ($(UNAME_S),Darwin)
INC=/Users/mam1/Documents/SimpleIDE/Learn/SimpleLibraries/Utility/libsimpletools 
INC+=/Users/mam1/Documents/SimpleIDE/Learn/SimpleLibraries/TextDevices/libsimpletext
INC+=/Users/mam1/Documents/SimpleIDE/Learn/SimpleLibraries/Protocol/libsimplei2c
INC+=/Users/mam1/Documents/SimpleIDE/Learn/SimpleLibraries/TextDevices/libfdserial
INC_PARAMS=$(foreach d, $(INC), -I$d)

LIBS= /Users/mam1/Documents/SimpleIDE/Learn/SimpleLibraries/Utility/libsimpletools/xmmc/libsimpletools.a
LIBS+=/Users/mam1/Documents/SimpleIDE/Learn/SimpleLibraries/TextDevices/libfdserial/lmm/libfdserial.a
LIBS+=/Users/mam1/Documents/SimpleIDE/Learn/SimpleLibraries/TextDevices/libsimpletext/lmm/libsimpletext.a
LIBS+= /Users/mam1/Documents/SimpleIDE/Learn/SimpleLibraries/Protocol/libsimplei2c/lmm/libsimplei2c.a
LIBNAME =  
else
LIBS = /Users/mam1/Documents/SimpleIDE/Learn/SimpleLibraries/Utility/libsimpletools/xmmc/libsimpletools.a
LIBNAME = 
endif

# where we installed the propeller binaries and libraries
PREFIX = /opt/parallax

ifndef MODEL
MODEL=lmm
endif

ifndef BOARD
BOARD=$(PROPELLER_LOAD_BOARD)
endif

ifneq ($(BOARD),)
BOARDFLAG=-b$(BOARD)
endif

ifneq ($(CHIP),)
CHIPFLAG = -m$(CHIP)
endif

CFLAGS_NO_MODEL := $(CFLAGS) $(CHIPFLAG)
CFLAGS += -m$(MODEL) $(CHIPFLAG)
CXXFLAGS += $(CFLAGS)
LDFLAGS += $(CFLAGS) -fno-exceptions -fno-rtti

ifneq ($(LDSCRIPT),)
LDFLAGS += -T $(LDSCRIPT)
endif

# basic gnu tools
CC = propeller-elf-gcc
CXX = propeller-elf-g++
LD = propeller-elf-ld
AS = propeller-elf-as
AR = propeller-elf-ar
OBJCOPY = propeller-elf-objcopy
LOADER = propeller-load
LOADER2 = p2load

# BSTC program
BSTC=bstc
SPINDIR=.

ifneq ($(NAME),)
$(NAME).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
endif

ifneq ($(LIBNAME),)
lib$(LIBNAME).a: $(OBJS)
	$(AR) rs $@ $(OBJS)
endif

%.o: %.c
	$(CC) -m32bit-doubles $(CFLAGS) $(INC_PARAMS)  -o $@ -c $<

%.o: %.cpp
	$(CC) $(CXXFLAGS) -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c $<

#
# a .cog program is an object file that contains code intended to
# run in a COG separate from the main program; i.e., it's a COG
# driver that the linker will place in the .text section.
#
%.cog: %.c
	$(CC) $(CFLAGS_NO_MODEL) -mcog -r -o $@ $<
	$(OBJCOPY) --localize-text --rename-section .text=$@ $@

%.cog: %.cogc
	$(CC) $(CFLAGS_NO_MODEL) -mcog -xc -r -o $@ $<
	$(OBJCOPY) --localize-text --rename-section .text=$@ $@

#
# a .ecog program is an object file that contains code intended to
# run in a COG separate from the main program; i.e., it's a COG
# driver that the linker will place in the .drivers section which
# gets loaded to high EEPROM space above 0x8000.
#
%.ecog: %.c
	$(CC) $(CFLAGS_NO_MODEL) -mcog -r -o $@ $<
	$(OBJCOPY) --localize-text --rename-section .text=$@ $@

%.ecog: %.ecogc
	$(CC) $(CFLAGS_NO_MODEL) -mcog -xc -r -o $@ $<
	$(OBJCOPY) --localize-text --rename-section .text=$@ $@

%.binary: %.elf
	$(LOADER) -s $<

%.dat: $(SPINDIR)/%.spin
	$(BSTC) -Ox -c -o $(basename $@) $<

%_firmware.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@

clean:
	rm -f *.o *.elf *.a *.cog *.ecog *.binary

#
# how to run
run: $(NAME).elf
ifeq ($(UNAME_S),Darwin)
	$(LOADER) $(BOARDFLAG) $(NAME).elf -r -t -p /dev/cu.usbserial-A4009G3O
else
	$(LOADER) $(BOARDFLAG) $(NAME).elf -r -t -p /dev/ttyUSB0
endif
run2: $(NAME).elf
	$(LOADER2) $(NAME).elf -t
#
