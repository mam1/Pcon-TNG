# scan directory 
program_NAME := read-log
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
	program_INCLUDE_DIRS := ../include ../../../BBBIOlib/BBBio_lib
	program_LIBRARY_DIRS := ../lib ../../../BBBIOlib/BBBio_lib
	program_LIBRARIES := Pcon BBBio
else
	program_INCLUDE_DIRS := ../include ../../../BBBIOlib/BBBio_lib
	program_LIBRARY_DIRS := ../lib ../../../BBBIOlib/BBBio_lib
	program_LIBRARIES := Pcon BBBio
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
	PREFIX := /usr/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin/arm-linux-gnueabihf-
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

#	$(CXX) -Wall -o test-ipc test-ipc.c -I../include ../lib/libPcon.a

	#-fno-exceptions -fno-rtti

	$(CXX) $(program_OBJS) -o $(program_NAME) -Wall -I../include -L../lib -lPcon  -Wall -fno-exceptions -fno-rtti
	
	# $(LINK.cc) $(program_OBJS) -o $(program_NAME)

# move the directory to the bone
# ifeq ($(UNAME_S),Darwin)
# 	rsync -avz  /Users/mam1/Git/Pcon-TNG/BBB/read-log/ root@192.168.254.34:/home/rsync-OSX
# else
# 	rsync -avz  /home/mam1/Git/Pcon-TNG/BBB/read-log/ root@192.168.254.34:/home/rsync-Mint	
# endif
# move the executable to /usr/local/bin/Pcon directory on the bone
	rsync -avz /Users/mam1/Git/Pcon-TNG/BBB/read-log/$(program_NAME) root@192.168.254.34:/usr/local/bin/Pcon
	ssh root@192.168.254.34 'chown root /usr/local/bin/Pcon/$(program_NAME)'		# change owner
	ssh root@192.168.254.34 'chmod 4755 /usr/local/bin/Pcon/$(program_NAME)'		# open access

# clean up
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS)

clean:
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS)

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
