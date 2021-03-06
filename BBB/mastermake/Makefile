#############################################################################
#	basic cross compile build
#	target - BeagleBone (Debian)														#
############################################################################# 

# set directories

program_DIR_PREFIX := /home/mam1/Git/Pcon-TNG
system_BINARIES := $(program_DIR_PREFIX)/bin
code_DIRECTORY := $(program_DIR_PREFIX)/BBB/$(working_DIRECTORY)
program_INCLUDE_DIRS := $(program_DIR_PREFIX)/BBB/include
program_LIBRARY_DIRS := $(program_DIR_PREFIX)/BBB/lib
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

	CC  := /home/mam1/gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
# 	CXX := ${CC}g++
# 	LD  := ${CC}ld
# 	AS  := ${CC}as
# 	AR  := ${CC}ar
# 	OBJCOPY := ${CC}objcopy

.PHONY: all clean distclean

# create executable - link in project library
all: $(program_NAME)

$(program_NAME): $(program_OBJS)
	@echo ${CC}
	@echo "\n>>>>> building $(program_NAME) sending output to $(target_MACHINE)"
	$(CC) $(program_OBJS) -o $(system_BINARIES)/$(program_NAME) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)
	@echo ">>>>> build of  $(program_NAME) completed"

# move the bins to the bone
# 	@echo "\n *** rsync -> $(program_DIR_PREFIX)/bin/ mam1@$(target_MACHINE):$(bone_BIN_DIR)\n"
	@echo ">>>>> moving binaries to $(target_MACHINE)"
	rsync -azv      $(program_DIR_PREFIX)/bin/ mam1@$(target_MACHINE):/usr/local/sbin

# move shell scripts to the bone
# 	printf "\n *** rsync -> $(program_DIR_PREFIX)/script_lib/Bone/ mam1@$target_MACHINE:$bone_BIN_DIR"
	@echo ">>>>> moving shell scripts to $(target_MACHINE)" 
	rsync -azv      $(program_DIR_PREFIX)/script_lib/Bone/ mam1@$(target_MACHINE):${bone_BIN_DIR}

# fix permissions
# 	ssh mam1@$(target_MACHINE) chown mam1:mam1 $(bone_BIN_DIR)/*	# change owner
# 	ssh mam1@$(target_MACHINE) chmod 4755 $(bone_BIN_DIR)/*		# open access

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