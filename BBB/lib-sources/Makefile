#
# File:  Makefile (for library)
#

program_C_SRCS := $(wildcard *.c)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_OBJS := $(program_C_OBJS)

# set directories
program_INCLUDE_DIRS := /home/mam1/Git/Pcon-TNG/BBB/include
program_LIBRARY_DIRS := /home/mam1/Git/Pcon-TNG/BBB/lib
program_LIBRARIES := libPcon

# set parameters
CPPFLAGS += -I/home/mam1/Git/Pcon-TNG/BBB/include
LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))
CFLAGS  += -Wall -I/home/mam1/Git/Pcon-TNG/BBB/include
CXXFLAGS += $(CFLAGS)
LDFLAGS += $(CFLAGS) -fno-exceptions -fno-rtti 

# set locations for basic gnu tools
PREFIX 	:= /home/mam1/gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
CC  	:= $(PREFIX)gcc
CXX 	:= $(PREFIX)g++
LD  	:= $(PREFIX)ld
AS  	:= $(PREFIX)as
AR  	:= $(PREFIX)ar
OBJCOPY := $(PREFIX)objcopy

# set libarary
LIB 	:= libPcon.so
LIBDEST := /home/mam1/Git/Pcon-TNG/BBB/lib
LIBSRC 	:= $(wildcard *.c)
LIBOBJ 	:= $(LIBSRC:.c=.o)

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
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo lib Makefile - cleaning up
	@- $(RM) -v $(LIBOBJ)


