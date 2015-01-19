# scan directory 
program_NAME := Pcon
program_C_SRCS := $(wildcard *.c)
program_CXX_SRCS := $(wildcard *.cpp)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS)

# get the development machine OS
UNAME_S := $(shell uname -s)
UNAME_P := $(shell uname -p)

# set directories
ifeq ($(UNAME_S),Darwin)
	program_INCLUDE_DIRS := 
	program_LIBRARY_DIRS := 
	program_LIBRARIES := 
else
	program_INCLUDE_DIRS := /usr/include 
	program_INCLUDE_DIRS += /usr/include/x86_64-linux-gnu
	# /usr/include/x86_64-linux-gnu/

	program_LIBRARY_DIRS := 
	program_LIBRARIES := 
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
	# PREFIX := /Volumes/arm-willtm-linux-gnueabi/bin/arm-linux-gnueabihf-
	PREFIX = /Volumes/arm-willtm-linux-gnueabi/bin/arm-linux-gnueabihf-
	# PREFIX := /opt/cross-compile/arm-willtm-linux-gnueabi/bin/arm-linux-gnueabihf-
	CXX := $(PREFIX)g++
	LD  := $(PREFIX)ld
	AS  := $(PREFIX)as
	AR  := $(PREFIX)ar
	OBJCOPY := $(PREFIX)objcopy
else
	# CFLAGS += -march=armv7-a -mtune=cortex-a8 -mfpu=neon -B/usr/lib32
	PREFIX := /usr/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin/arm-linux-gnueabihf-
	CC  := $(PREFIX)gcc
	CXX := $(PREFIX)gcc
	LD  := $(PREFIX)ld
	AS  := $(PREFIX)as
	AR  := $(PREFIX)ar
	OBJCOPY := $(PREFIX)objcopy
endif

.PHONY: all clean distclean

all: $(program_NAME)

$(program_NAME): $(program_OBJS)
	$(LINK.cc) $(program_OBJS)-o $(program_NAME)

# move the binary to the bone
ifeq ($(UNAME_S),Darwin)
	rsync -avz  /Users/mam1/Git/Pcon-TNG/Pcon-BBB/ root@192.168.254.34:/home/rsync-OSX
else
	rsync -avz  /home/mam1/Git/Pcon-TNG/Pcon-BBB/ root@192.168.254.34:/home/rsync-Mint	
endif

# clean up
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS)

clean:
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS)

distclean: clean
