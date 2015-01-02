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
CFLAGS  += -Wall
CXXFLAGS += $(CFLAGS)
LDFLAGS += $(CFLAGS) -fno-exceptions -fno-rtti 

#get the development machine OS
UNAME_S := $(shell uname -s)
UNAME_P := $(shell uname -p)

# set up locations for basic gnu tools
ifeq ($(UNAME_S),Darwin)
	PREFIX = /Volumes/arm-willtm-linux-gnueabi/bin/arm-linux-gnueabihf-
	CC  := $(PREFIX)gcc 
	CXX := $(PREFIX)g++
	LD  := $(PREFIX)ld
	AS  := $(PREFIX)as
	AR  := $(PREFIX)ar
	OBJCOPY := $(PREFIX)objcopy
else
	PREFIX = /usr/bin
	CC  := arm-linux-gnueabihf-gcc
	CXX := arm-linux-gnueabihf-g++
	LD  := arm-linux-gnueabihf-ld
	AS  := arm-linux-gnueabihf-as
	AR  := arm-linux-gnueabihf-ar
	OBJCOPY := arm-linux-gnueabihf-objcopy
endif

.PHONY: all clean distclean

all: $(program_NAME)

$(program_NAME): $(program_OBJS)
	$(LINK.cc) $(program_OBJS)-o $(program_NAME)

ifeq ($(UNAME_S),Darwin)
	rsync -avz  /Users/mam1/Git/Pcon-TNG/Pcon-BBB root@192.168.254.34:/home/rsync-Pcon
else
	cp -v /home/mam1/Git/Pcon-TNG/Pcon-BBB/$(program_NAME) /home/mam1/BBB
endif
	@- $(RM) $(program_NAME)
	@- $(RM) $(program_OBJS)

clean:
	@- $(RM) $(program_NAME)
	@- $(RM) $(program_OBJS)

distclean: clean
