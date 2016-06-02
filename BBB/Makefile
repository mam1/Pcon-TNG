# File:  Makefile (for library)
#

program_C_SRCS := $(wildcard *.c)
program_C_SRCS := $(wildcard *.c)
program_CXX_SRCS := $(wildcard *.cpp)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS)

# get the development machine OS
UNAME_S := $(shell uname -s)
UNAME_P := $(shell uname -p)

# set directories
# ifeq ($(UNAME_S),Darwin)
program_INCLUDE_DIRS := include
program_LIBRARY_DIRS := lib
program_LIBRARIES := Pcon.a
# else
# 	program_INCLUDE_DIRS := include
# 	program_LIBRARY_DIRS := lib
# 	program_LIBRARIES := Pcon.a
# endif

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
	PREFIX := /home/mam1/X-comp/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin/arm-linux-gnueabihf-
	CC  := $(PREFIX)gcc
	CXX := $(PREFIX)g++
	LD  := $(PREFIX)ld
	AS  := $(PREFIX)as
	AR  := $(PREFIX)ar
	OBJCOPY := $(PREFIX)objcopy
endif

# set libarary
LIB=libPcon.a
LIBDEST= lib
LIBSRC := $(wildcard lib-sources/*.c)
LIBOBJ=$(LIBSRC:.c=.o)

install: $(LIB)
	@echo lib Makefile - LIBOBJ $(LIBOBJ)
	@echo lib Makefile - libsrc $(LIBSRC)
	@echo lib Makefile - installing  $(LIB)
	@echo lib Makefile - into $(LIBDEST)
	@install -m 444 $(LIB) $(LIBDEST)

$(LIB): $(LIBOBJ)
	@echo lib Makefile - archiving $(LIB)
	@$(AR) r $(LIB) $(LIBOBJ)

.c.o:
	@echo lib Makefile - compiling $<
	@$(CC) $(CFLAGS) -Iinclude -c $< -o $@

clean:
	@echo lib Makefile - cleaning up
	@- $(RM) -v $(LIBOBJ)


