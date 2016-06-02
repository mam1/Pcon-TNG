# get the development machine OS
UNAME_S := $(shell uname -s)

# set locations for basic gnu tools based on the deveopment machine OS
ifeq ($(UNAME_S),Darwin)
	PREFIX := /Volumes/cross-compile/bin/arm-linux-gnueabihf-
else
	PREFIX := /home/mam1/X-comp/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin/arm-linux-gnueabihf-
endif
CC  := $(PREFIX)gcc
CXX := $(PREFIX)g++
LD  := $(PREFIX)ld
AS  := $(PREFIX)as
AR  := $(PREFIX)ar
OBJCOPY := $(PREFIX)objcopy
	
program_NAME := propd
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

.PHONY: all clean distclean

all: $(program_NAME)

$(program_NAME): $(program_OBJS)
	$(LINK.cc) $(program_OBJS) -o $(program_NAME)
	cp propd ../Pcon-BBB/propd

clean:
	@- $(RM) $(program_NAME)
	@- $(RM) $(program_OBJS)

distclean: clean