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
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
endif

ifneq ($(LIBNAME),)
lib$(LIBNAME).a: $(OBJS)
	$(AR) rs $@ $(OBJS)
endif

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CC) $(CXXFLAGS) -o $@ -c $<

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
