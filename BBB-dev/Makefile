program_NAME := Pcon
program_C_SRCS := $(wildcard *.c)
program_CXX_SRCS := $(wildcard *.cpp)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS)
program_INCLUDE_DIRS :=
program_LIBRARY_DIRS :=
program_LIBRARIES :=

CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))

# CFLAGS_NO_MODEL := $(CFLAGS) $(CHIPFLAG)
# CFLAGS += -m$(MODEL) $(CHIPFLAG)
CXXFLAGS += $(CFLAGS)
LDFLAGS += $(CFLAGS) -fno-exceptions -fno-rtti

# basic gnu tools
PREFIX = /usr/bin
CC := arm-linux-gnueabihf-gcc
CXX := arm-linux-gnueabihf-g++
LD := arm-linux-gnueabihf-ld
AS := arm-linux-gnueabihf-as
AR := arm-linux-gnueabihf-ar
OBJCOPY := arm-linux-gnueabihf-objcopy

# LOADER := propeller-load
# LOADER2 := p2load

.PHONY: all clean distclean

all: $(program_NAME)

$(program_NAME): $(program_OBJS)
	$(LINK.cc) $(program_OBJS) -o $(program_NAME)
	cp proggram_name /home/mam1/BBB

clean:
	@- $(RM) $(program_NAME)
	@- $(RM) $(program_OBJS)

distclean: clean

