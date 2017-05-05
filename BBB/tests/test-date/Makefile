


program_NAME := test-date

# scan directory 
program_C_SRCS := $(wildcard *.c)
program_CXX_SRCS := $(wildcard *.cpp)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS)

# get the development machine OS
UNAME_S := $(shell uname -s)

# set directories
ifeq ($(UNAME_S),Darwin)
	program_INCLUDE_DIRS := ../../include
	program_LIBRARY_DIRS := ../../lib
	program_LIBRARIES := :libPcon.a
else
	program_INCLUDE_DIRS := ../include
	program_LIBRARY_DIRS := ../lib
	program_LIBRARIES := libPcon
endif

# set parameters
CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))
CFLAGS  += -Wall
CXXFLAGS += $(CFLAGS)
LDFLAGS += $(CFLAGS) -fno-exceptions -fno-rtti 

# get the host os 
UNAME_S := $(shell uname -s)

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
	PREFIX := /gcc-linaro-5.4.1-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
	CC  := $(PREFIX)gcc
	CXX := $(PREFIX)g++
	LD  := $(PREFIX)ld
	AS  := $(PREFIX)as
	AR  := $(PREFIX)ar
	OBJCOPY := $(PREFIX)objcopy
endif

.PHONY: all clean distclean

all: $(program_NAME)

$(program_NAME): $(program_OBJS)

#	$(LINK.cc) $(program_OBJS)-o $(program_NAME)
	# $(CXX) $(program_OBJS) -o $(program_NAME) -Wall -I../../include  -Wall -fno-exceptions -fno-rtti 
	$(CC) $(program_OBJS) -o $(program_NAME) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

# move the directory to the bone
ifeq ($(UNAME_S),Darwin)
	rsync -avz  /Users/mam1/Git/Pcon-TNG/BBB/tests/test-date/test-date root@192.168.254.34:/usr/local/sbin/test-date
else
	rsync -avz  /home/mam1/Git/Pcon-TNG/BBB/tests/test-date/test-date root@192.168.254.34:/usr/local/sbin/test-date
endif

	ssh root@192.168.254.34 /usr/local/sbin/test-date		# change owner
	ssh root@192.168.254.34 /usr/local/sbin/test-date		# open access
	
# clean up
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS)

clean:
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS)

# rebuild the project ibrary from lib-sources 
lib:
	cd ../.. && $(MAKE) clean
	cd ../.. && $(MAKE)

dump:
	@echo program_NAME $(program_NAME)
	@echo program_OBJS $((program_OBJS)
	@echo LDFLAGS $(LDFLAGS)
	@echo CPPFLAGS $(CPPFLAGS)
	@echo CFLAGS $(CFLAGS)
	@echo LINK.cc $(LINK.cc)

	@echo CC ............... $(CC)
	@echo cc ............... $(cc)
	@echo LINK ............. $(LINK)
	@echo LINK.cc .......... $(LINK.cc)
	@echo program_NAME ..... $(program_NAME)
	@echo program_C_OBJS ... $(program_C_OBJS)
	@echo program_OBJS ..... $(program_OBJS)
	@echo LDFLAGS .......... $(LDFLAGS)
	@echo CPPFLAGS ......... $(CPPFLAGS)
	@echo CFLAGS ........... $(CFLAGS)


distclean: clean
