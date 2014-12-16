

CFLAGS = -Os -m32bit-doubles -Wall
NM = propeller-elf-nm
MODEL = lmm
BOARD = QUICKSTART
# LDF                                           LAGS = -I
# LIBS := simpletools simpletext libfdserial
# LIBNAME := /home/mam1/SimpleIDE/Learn/SimpleLibraries/Utility/libsimpletools/$(MODEL)
# LIBNAME += /home/mam1/SimpleIDE/Learn/SimpleLibraries/TextDevices/libfdserial/$(MODEL)
# LIBNAME += /home/mam1/SimpleIDE/Learn/SimpleLibraries/TextDevices/libsimpletext/$(MODEL)
# LIBNAME += /home/mam1/SimpleIDE/Learn/SimpleLibraries/Protocol/libsimplei2c/$(MODEL)

# objects for this program
NAME = Pcon
OBJS = Pcon.o fdserial.o fdserial_utils.o simpleterm_close.o
program_NAME := Pcon
program_C_SRCS := $(wildcard *.c)
program_COG_SRCS := $(wildcard *.cogc)
program_CXX_SRCS := $(wildcard *.cpp)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_COG_OBJS := ${program_COG_SRCS:.cogc=.o}
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS)
# OBJS := program_OBJS

program_INCLUDE_DIRS := . 
program_INCLUDE_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/Utility/libsimpletools   
program_INCLUDE_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/TextDevices/libfdserial  
program_INCLUDE_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/TextDevices/libsimpletext
program_INCLUDE_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/Protocol/libsimplei2c

program_LIBRARY_DIRS := . 
program_LIBRARY_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/Utility/libsimpletools/$(MODEL)
program_LIBRARY_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/TextDevices/libfdserial/$(MODEL)
program_LIBRARY_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/TextDevices/libsimpletext/$(MODEL)
program_LIBRARY_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/Protocol/libsimplei2c/$(MODEL)

program_LIBRARIES := simpletools simpletext

INCLUDE_DIRS += $(foreach includedir,$(program_INCLUDE_DIRS),-I $(includedir))
LIBRARY_DIRS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L $(librarydir))
LIBRARIES += $(foreach library,$(program_LIBRARIES),-l$(library))

.PHONY: all clean distclean

all: $(NAME).elf

include common.mk


# #########################################################
#   
#  NAME to be the name of project
#       - this is used to create the final program $(NAME).elf
#  OBJS to be the object files needed for the project
#  MODEL to lmm, xmm, or xmmc
#  CFLAGS to be desired CFLAGS
#
# #########################################################

# where we installed the propeller binaries and libraries
PREFIX = /opt/parallax

ifndef MODEL
MODEL=xmmc
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
LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))

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

# SPIN program
SPIN=bstc
SPINDIR=.

ifneq ($(NAME),)
$(NAME).elf: $(OBJS)
	$(CC) $(INCLUDE_DIRS) $(LDFLAGS) -o $@ $(OBJS) 
endif

ifneq ($(LIBNAME),)
lib$(LIBNAME).a: $(OBJS)
	$(AR) rs $@ $(OBJS)
endif

%.o: %.c
	$(CC) $(INCLUDE_DIRS) $(CFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CC) $(INCLUDE_DIRS) $(CXXFLAGS) -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c -M $<

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
	$(SPIN) -Ox -c -o $(basename $@) $<

%_firmware.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@

clean:
	rm -f *.o *.elf *.a *.cog *.ecog *.binary

#
# how to run
run: $(NAME).elf
	$(LOADER) $(BOARDFLAG) $(NAME).elf -r -t

r: $(NAME).elf
	$(LOADER) $(BOARDFLAG) $(NAME).elf -r -t -p /dev/cu.usbserial-004213FA

e: $(NAME).elf
	$(LOADER) $(BOARDFLAG) $(NAME).elf -r -t -e -p /dev/cu.usbserial-004213FA

run2: $(NAME).elf
	$(LOADER2) $(NAME).elf -t
#
