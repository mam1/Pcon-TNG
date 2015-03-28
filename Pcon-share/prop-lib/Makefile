

# CFLAGS = -Os -m32bit-doubles -Wall -std=c99 
# # LDFLAGS = -I /Users/mam1/Documents/SimpleIDE/Learn/SimpleLibraries/Utility/libsimpletools
# LDFLAGS = -I /Users/mam1/Documents/SimpleIDE/Learn/SimpleLibraries/Utility/libsimpletools 
# NM = propeller-elf-nm
# # MODEL = xmmc
# MODEL = lmm
# BOARD =  QUICKSTART


# #CFLAGS = -Os -mlmm
# CFLAGS = -Os
# NM = propeller-elf-nm

# #
# # objects for this program
# #

# NAME = Pcon
# OBJS = Pcon.o ../Pcon-share/prop-lib/schedule.o

# basic gnu tools
GCC_PREFIX = /opt/parallax/bin
CC = $(GCC_PREFIX)/propeller-elf-gcc
AR = $(GCC_PREFIX)/propeller-elf-ar

all: schedule
schedule: 
	$(CC) -Wall -c ../lib-sources/schedule.c 
	$(AR) rcs bone-lib.a schedule.o