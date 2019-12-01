#############################################################################
#	basic cross compile build
#	target - BeagleBone (Debian)												
############################################################################# 
# 
# set directories
system_BINARIES := $(program_DIR_PREFIX)/bin
code_DIRECTORY := $(program_DIR_PREFIX)/BBB/$(working_DIRECTORY)
program_INCLUDE_DIRS := $(program_DIR_PREFIX)/BBB/include $(program_INCLUDE_DIRS) /usr/include/x86_64-linux-gnu/
program_LIBRARY_DIRS := $(program_DIR_PREFIX)/BBB/lib /tmp/openssl-1.0.1t/libssl.a /tmp/openssl-1.0.1t/libssl.a /home/mam1/Git/paho.mqtt.c/build/output
program_LIBRARIES := Pcon

# scan working directory
program_C_SRCS := $(wildcard *.c)
program_CXX_SRCS :=  $(wildcard *.cpp)
program_C_OBJS :=  ${program_C_SRCS:.c=.o}
program_CXX_OBJS :=  ${program_CXX_SRCS:.cpp=.o}
program_OBJS :=  $(program_C_OBJS) $(program_CXX_OBJS)

# set parameters
CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))
CFLAGS  += -Wall
CXXFLAGS += $(CFLAGS)
LDFLAGS += $(CFLAGS) -fno-exceptions      

# set locations for basic gnu tools
	PREFIX := /home/mam1/gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
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
	@echo "\n>>>>> building $(program_NAME)"
	$(CC) $(program_OBJS) -o $(system_BINARIES)/$(program_NAME) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)
	@echo ">>>>> build of $(program_NAME) completed"

# move the bins to the bone
	@echo ">>>>> copying binaries to device at $(target_MACHINE)"
# 	@echo "\n *** rsync -> $(program_DIR_PREFIX)/bin/ mam1@$(target_MACHINE):$(bone_BIN_DIR)\n"
	rsync -azv $(system_BINARIES)/ mam1@$(target_MACHINE):$(bone_BIN_DIR)
	@echo "\n"

# move the scripts to the bone
	@echo ">>>>> copying scripts to device at $(target_MACHINE)"
# 	@echo "\n *** rsync -> $(program_DIR_PREFIX)/bin/ mam1@$(target_MACHINE):$(bone_BIN_DIR)\n"
	rsync -azv /home/mam1/Git/Pcon-TNG/script_lib/Bone/ mam1@$(target_MACHINE):$(bone_BIN_DIR)
	@echo "\n"

# if the progeam is a daemon move shell script to /etc/init.d bone
# 	@if [ "$(strip $(program_TYPE))" =  daemon ]; then

# 		@echo ">>>>> copying init.d script to device at $(target_MACHINE)\n"
# 		# 		@echo "\n *** rsync -> $(program_DIR_PREFIX)/script_lib/daemon/ mam1@$(target_MACHINE):/etc/init.d\n" 
# 		rsync -azv      $(program_DIR_PREFIX)/script_lib/daemon/ root@$(target_MACHINE):/etc/init.d
# 		@echo "\n"	
# 	fi

# fix permissions
	ssh mam1@$(target_MACHINE) sudo chown mam1:mam1 $(bone_BIN_DIR)/*	# change owner
	ssh mam1@$(target_MACHINE) sudo chmod 777 $(bone_BIN_DIR)/*		# open acces


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
	cd ../CGI && $(MAKE)
	cd ../UI && $(MAKE) t=$(target_MACHINE)
	cd ../daemon && $(MAKE)	t=$(target_MACHINE)
	cd ../../script_lib/OSXMint && $(MAKE)
	sload $(target_MACHINE)

dump:
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

distclean: cleans