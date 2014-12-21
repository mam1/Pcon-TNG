

CFLAGS = -Os -m32bit-doubles -Wall -std=c99 
NM = propeller-elf-nm
MODEL = lmm
BOARD =  QUICKSTART
# LDF                                           LAGS = -I
# LIBS = /home/mam1/SimpleIDE/Learn/SimpleLibraries/TextDevices/libsimpletext/lmm/libsimpletext.a
# LIBS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/TextDevices/libsimpletext/$(MODEL)/
# LIBS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/Protocol/libsimplei2c/$(MODEL)/

# LIBNAME = simpletools
# LIBNAME += libfdserial
# LIBNAME += simpletext 
# LIBNAME += 

# objects for this program
NAME = Pcon
OBJS = Pcon.o 

#fdserial.o fdserial_utils.o simpleterm_close.o

HACK = -I . -L . 
HACK += -I ./../Learn/'Simple Libraries'/TextDevices/libfdserial -L ./../Learn/'Simple Libraries'/TextDevices/libfdserial/lmm/ 
HACK += -I /home/mam1/SimpleIDE/Learn/'Simple Libraries'/TextDevices/libfdserial -L /home/mam1/SimpleIDE/Learn/'Simple Libraries'/TextDevices/libfdserial/lmm/ 
HACK += -I /home/mam1/SimpleIDE/Learn/'Simple Libraries'/TextDevices/libsimpletext -L /home/mam1/SimpleIDE/Learn/'Simple Libraries'/TextDevices/libsimpletext/lmm/ 
HACK += -I /home/mam1/SimpleIDE/Learn/'Simple Libraries'/Utility/libsimpletools -L /home/mam1/SimpleIDE/Learn/'Simple Libraries'/Utility/libsimpletools/lmm/ 
HACK += -I /home/mam1/SimpleIDE/Learn/'Simple Libraries'/Protocol/libsimplei2c -L /home/mam1/SimpleIDE/Learn/'Simple Libraries'/Protocol/libsimplei2c/lmm/

HACK2 = -lm -lfdserial -lsimpletext -lsimpletools -lsimplei2c -lm -lfdserial -lsimpletext -lsimpletools -lm -lfdserial -lsimpletext -lm -lfdserial -lm


# program_NAME = Pcon
# program_C_SRCS = $(wildcard *.c)
# program_COG_SRCS = $(wildcard *.cogc)
# program_CXX_SRCS = $(wildcard *.cpp)
# program_C_OBJS = ${program_C_SRCS:.c=.o}
# program_COG_OBJS = ${program_COG_SRCS:.cogc=.o}
# program_CXX_OBJS = ${program_CXX_SRCS:.cpp=.o}
# program_OBJS = $(program_C_OBJS) $(program_CXX_OBJS)
# OBJS = program_OBJS

# program_INCLUDE_DIRS = . 
# program_INCLUDE_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/Utility/libsimpletools   
# program_INCLUDE_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/TextDevices/libfdserial  
# program_INCLUDE_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/TextDevices/libsimpletext
# program_INCLUDE_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/Protocol/libsimplei2c

# program_LIBRARY_DIRS = . 
# program_LIBRARY_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/Utility/libsimpletools/$(MODEL)
# program_LIBRARY_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/TextDevices/libfdserial/$(MODEL)
# program_LIBRARY_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/TextDevices/libsimpletext/$(MODEL)
# program_LIBRARY_DIRS += /home/mam1/SimpleIDE/Learn/SimpleLibraries/Protocol/libsimplei2c/$(MODEL)

# program_LIBRARIES = simpletools simpletext

# INCLUDE_DIRS += $(foreach includedir,$(program_INCLUDE_DIRS),-I $(includedir))
# LIBRARY_DIRS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L $(librarydir))
# LIBRARIES += $(foreach library,$(program_LIBRARIES),-l$(library))

.PHONY: all clean distclean

all: $(NAME).elf

include common.mk
