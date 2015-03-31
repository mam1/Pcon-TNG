
# get the development machine OS
UNAME_S := $(shell uname -s)

# set directories
ifeq ($(UNAME_S),Darwin)
	program_INCLUDE_DIRS := ../Pcon-share/
	program_LIBRARY_DIRS := 
	program_LIBRARIES := 
	LD_LIBRARY_PATH := 
else
	program_INCLUDE_DIRS := ../Pcon-share
	program_LIBRARY_DIRS := 
	program_LIBRARIES := 
	LD_LIBRARY_PATH := /home/mam1/Git/Pcon-TNG/Pcon-share/bone-lib
endif

# set parameters
CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))
CFLAGS  += -Wall
CXXFLAGS += $(CFLAGS)
LDFLAGS += $(CFLAGS) -fno-exceptions -fno-rtti 

# set locations for basic gnu tools
ifeq ($(UNAME_S),Darwin)
	PREFIX := /Volumes/cross-compile/bin/arm-linux-gnueabihf-
	CC  := $(PREFIX)gcc
	CXX := $(PREFIX)g++
	LD  := $(PREFIX)ld
	AS  := $(PREFIX)as
	AR  := $(PREFIX)ar
	OBJCOPY := $(PREFIX)objcopy
else
	PREFIX := /usr/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin/arm-linux-gnueabihf-
	CC  := $(PREFIX)gcc
	CXX := $(PREFIX)g++
	LD  := $(PREFIX)ld
	AS  := $(PREFIX)as
	AR  := $(PREFIX)ar
	OBJCOPY := $(PREFIX)objcopy
endif

all:
	$(CC) -Wall -c ../lib-sources/schedule.c 
	$(AR) -cvr libbone.a schedule.o
	# curret library contents
	$(AR) -t libbone.a
	ranlib libbone.a
