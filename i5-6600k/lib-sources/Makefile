#
# File:  Makefile (for library)
#

program_C_SRCS := $(wildcard *.c)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_OBJS := $(program_C_OBJS) 

# set directories
program_INCLUDE_DIRS := /home/mam1/Git/Pcon-TNG/i5-6600k/include
program_LIBRARY_DIRS := /home/mam1/Git/Pcon-TNG/i5-6600k/lib
program_LIBRARIES := libi5-6600k

# set parameters
# CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
CPPFLAGS := -I/home/mam1/Git/Pcon-TNG/i5-6600k/include

LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))
CFLAGS  += -Wall -I/home/mam1/Git/Pcon-TNG/i5-6600k/include
CXXFLAGS += $(CFLAGS)
LDFLAGS += $(CFLAGS) 

# set libarary
LIB=libi5-6600k.so
LIBDEST= /home/mam1/Git/Pcon-TNG/i5-6600k/lib
LIBSRC := $(wildcard *.c)
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
	cc $(CFLAGS) -Iprogram_INCLUDE_DIRS -c $< -o $@

clean:
	@echo lib Makefile - cleaning up
	@- $(RM) -v $(LIBOBJ)


