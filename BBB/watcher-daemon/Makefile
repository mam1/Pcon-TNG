ifndef t
$(error target for make not set)
endif

working_DIRECTORY:=watcher-daemon 
program_NAME:=watcher-d 
target_MACHINE:=$t

include ~/Git/Pcon-TNG/BBB/mastermake/Makefile








# #############################################################################
# #	basic cross compile build
# #	target - BeagleBone (Debian)														#
# ############################################################################# 

# program_NAME := Dcon

# # scan directory
# program_C_SRCS := $(wildcard *.c)
# program_CXX_SRCS := $(wildcard *.cpp)
# program_C_OBJS := ${program_C_SRCS:.c=.o}
# program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
# program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS)

# # get the development machine OS
# UNAME_S := $(shell uname -s)


# # set directories
# ifeq ($(UNAME_S),Darwin)
# 	program_DIR_PREFIX := /Users/mam1/Git/Pcon-TNG
# else
# 	program_DIR_PREFIX := /home/mam1/Git/Pcon-TNG
# endif
	

# system_BINARIES := $(program_DIR_PREFIX)/bin
# program_INCLUDE_DIRS := $(program_DIR_PREFIX)/BBB/include
# program_LIBRARY_DIRS := $(program_DIR_PREFIX)/BBB/lib
# program_LIBRARIES := Pcon

# # set parameters
# CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
# LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
# LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))
# CFLAGS  += -Wall
# CXXFLAGS += $(CFLAGS)
# LDFLAGS += $(CFLAGS) -fno-exceptions      

# # set locations for basic gnu tools
# ifeq ($(UNAME_S),Darwin)
# 	PREFIX := /Volumes/cross-compile/bin/arm-linux-gnueabihf-
# 	CC  := $(PREFIX)gcc
# 	CXX := $(PREFIX)g++
# 	LD  := $(PREFIX)ld
# 	AS  := $(PREFIX)as
# 	AR  := $(PREFIX)ar
# 	OBJCOPY := $(PREFIX)objcopy
# else
# 	CC=/gcc-linaro-5.4.1-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
# 	# PREFIX := `pwd`/gcc-linaro-5.4.1-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
# 	CC  := $(CC)gcc
# 	CXX := $(CC)g++
# 	LD  := $(CC)ld
# 	AS  := $(CC)as
# 	AR  := $(CC)ar
# 	OBJCOPY := $(CC)objcopy
# endif

# .PHONY: all clean distclean

# # create executable - link in project library
# all: $(program_NAME)

# $(program_NAME): $(program_OBJS)
# 	@echo "\n$(UNAME_S)\n"
# 	@echo "$(program_DIR_PREFIX)\n"
# 	$(CC) $(program_OBJS) -o $(system_BINARIES)/$(program_NAME) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

# # move the executable to the bone
# 	bload dev

# # clean up
# 	@- $(RM) -v $(program_NAME)
# 	@- $(RM) -v $(program_OBJS)

# clean:
# 	@- $(RM) -v $(program_NAME)
# 	@- $(RM) -v $(program_OBJS)
# 	cd .. && $(MAKE) clean

# # rebuild the project ibrary from lib-sources 
# lib: 
# 	cd .. && $(MAKE) clean
# 	cd .. && $(MAKE)

# # build all project moduals
# project:
# 	cd .. && $(MAKE) clean
# 	cd .. && $(MAKE)
# 	cd ../CGI && $(MAKE)
# 	cd ../UI && $(MAKE)
# 	cd ../daemon && $(MAKE)	

# dump:
# 	@echo CC ............... $(CC)
# 	@echo cc ............... $(cc)
# 	@echo LINK ............. $(LINK)
# 	@echo LINK.cc .......... $(LINK.cc)
# 	@echo program_NAME ..... $(program_NAME)
# 	@echo program_C_OBJS ... $(program_C_OBJS)
# 	@echo program_OBJS ..... $(program_OBJS)
# 	@echo LDFLAGS .......... $(LDFLAGS)
# 	@echo CPPFLAGS ......... $(CPPFLAGS)
# 	@echo CFLAGS ........... $(CFLAGS)

# distclean: cleans