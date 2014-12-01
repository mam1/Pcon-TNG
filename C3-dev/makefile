
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
program_COG_SRCS := $(wildcard *.cogc)
program_CXX_SRCS := $(wildcard *.cpp)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_COG_OBJS := ${program_COG_SRCS:.cogc=.o}
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
	@echo "starting compile\n"
	@echo "CPPFLAGS: " $(CPPFLAGS) '\n'
	@echo "LDFLAGS: " $(LDFLAGS) '\n'
	@echo "C sources " $(program_C_SRCS)
	@echo "C cog sources " $(program_COG_SRCS)
	@echo "C objects " $(program_C_OBJS)
	@echo "COG objects " $(program_COG_OBJS)	
	@echo "program objects " $(program_OBJS)
	
include common.mk
