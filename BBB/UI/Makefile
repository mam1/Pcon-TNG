#############################################################################
#	basic build																#
############################################################################# 

# scan directory 
program_NAME := Pcon
program_C_SRCS := $(wildcard *.c)
program_CXX_SRCS := $(wildcard *.cpp)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS)

# get the development machine OS
UNAME_S := $(shell uname -s)

# set directories
ifeq ($(UNAME_S),Darwin)
	program_DIR := /Users/mam1/Git/Pcon-TNG/BBB/UI
	program_INCLUDE_DIRS := ../include
	program_LIBRARY_DIRS := ../lib
	program_LIBRARIES := Pcon
else
	program_DIR := /home/mam1/Git/Pcon-TNG/BBB/UI
	program_INCLUDE_DIRS := ../include
	program_LIBRARY_DIRS := ../lib
	program_LIBRARIES := Pcon
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
else
	PREFIX := /home/mam1/X-comp/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
endif
	CC  := $(PREFIX)gcc
	CXX := $(PREFIX)g++
	LD  := $(PREFIX)ld
	AS  := $(PREFIX)as
	AR  := $(PREFIX)ar
	OBJCOPY := $(PREFIX)objcopy

.PHONY: all clean distclean

# create executable - link in project library
all: $(program_NAME)

$(program_NAME): $(program_OBJS)
	$(CXX) $(program_OBJS) -o $(program_NAME) -Wall -I../include -L../lib -lPcon -Wall -fno-exceptions -fno-rtti

# move the executable to the bone
	rsync -avz  $(program_DIR)/$(program_NAME) root@192.168.254.34:/usr/local/sbin 

# fix permissions
	ssh root@192.168.254.34 'chown root:root /usr/local/sbin/$(program_NAME)'	# change owner
	ssh root@192.168.254.34 'chmod 4755 /usr/local/sbin/$(program_NAME)'		# open access

# clean up
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS)

clean:
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS)
	cd .. && $(MAKE) clean

# rebuild the project ibrary from lib-sources 
lib:
	cd .. && $(MAKE) clean
	cd .. && $(MAKE)

# build all project moduals
project:
	cd .. && $(MAKE) clean
	cd .. && $(MAKE)
	# cd ../CGI && $(MAKE)
	cd ../UI && $(MAKE)
	cd ../daemon && $(MAKE)	

dump:
	@echo CC $(CC)
	@echo cc $(cc)
	@echo LINK $(LINK)
	@echo LINK.cc $(LINK.cc)
	@echo program_NAME $(program_NAME)
	@echo program_C_OBJS $(program_C_OBJS)
	@echo program_OBJS $(program_OBJS)
	@echo LDFLAGS $(LDFLAGS)
	@echo CPPFLAGS $(CPPFLAGS)
	@echo CFLAGS $(CFLAGS)
	@echo LINK.cc $(LINK.cc)


distclean: clean