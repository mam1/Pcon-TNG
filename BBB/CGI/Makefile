program_NAME := Scon
target_ip := 192.168.254.72
cgi_dir := /usr/lib/cgi-bin

# scan directory 
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
	program_INCLUDE_DIRS := ../include
	program_LIBRARY_DIRS := ../lib
	program_LIBRARIES := libPcon
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

# set the local CGI directory
ifeq ($(UNAME_S),Darwin)
	LOCAL_CGI_DIR := /Users/mam1/Git/Pcon-TNG/BBB/CGI/Scon
else
	LOCAL_CGI_DIR := /home/mam1/Git/Pcon-TNG/BBB/CGI/Scon 
endif


.PHONY: all clean distclean

all: $(program_NAME)

$(program_NAME): $(program_OBJS)

#	$(LINK.cc) $(program_OBJS)-o $(program_NAME)
	$(CXX) $(program_OBJS) -lm -o $(program_NAME) -Wall -I../include -L../lib -lPcon -Wall -fno-exceptions -fno-rtti 

# move the executable to cgi directory on the bone
	rsync -avz  $(LOCAL_CGI_DIR) root@$(target_ip):$(cgi_dir)
	ssh root@$(target_ip) 'chown root $(cgi_dir)/Scon'		# change owner
	ssh root@$(target_ip) 'chmod 4755 $(cgi_dir)/Scon'		# open access

# clean up
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS)

clean:
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS)

dump:
	@echo program_NAME $(program_NAME)
	@echo program_OBJS $((program_OBJS)
	@echo LDFLAGS $(LDFLAGS)
	@echo CPPFLAGS $(CPPFLAGS)
	@echo CFLAGS $(CFLAGS)
	@echo LINK.cc $(LINK.cc)
	@echo LOGAL_CGI_DIT $(LOCAL_CGI_DIR)

distclean: clean
