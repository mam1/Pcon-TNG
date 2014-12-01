
#PROPLIB = common
CFLAGS = -Os -m32bit-doubles
NM = propeller-elf-nm
MODEL = lmm
BOARD = QUICKSTART
#LDFLAGS = -I
#LIBS = '/Users/mam1/Documents/SimpleIDE/Learn/Simple Libraries/Text Devices/libsimpletext'
#LIBNAME = /Users/mam1/Documents/SimpleIDE/Learn/Simple Libraries/Utility/libsimpletools


# objects for this program
NAME = main
OBJS = main.o cogA.cog cogB.cog cogC.cog

program_NAME := Pcon
program_C_SRCS := $(wildcard *.c)
program_CXX_SRCS := $(wildcard *.cpp)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS)


program_INCLUDE_DIRS := /Simple_Libraries/Text_Devices/libsimpletext /Simple_Libraries/Text_Devices/libfdserial
program_LIBRARY_DIRS := /Simple_Libraries/Text_Devices/libsimpletext/lmm /Simple_Libraries/Text_Devices/libserial/lmm
program_LIBRARIES := simpletext fdserial simpletools 

CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))

.PHONY: all clean distclean

all: $(NAME).elf
	@echo "starting compile"
	@echo $(CPPFLAGS)
	@echo $(LDFLAGS)
	

include common.mk
