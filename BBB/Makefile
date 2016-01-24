# File:  Makefile (for library)
#
CC=gcc
LIB=libPcon.a
LIBDEST=./

LIBSRC=abc.c def.c

LIBOBJ=$(LIBSRC:.c=.o)

install: $(LIB)
	@echo lib Makefile - installing $(LIB)
	@install -m 444 $(LIB) $(LIBDEST)

$(LIB): $(LIBOBJ)
	@echo lib Makefile - archiving $(LIB)
	@$(AR) r $(LIB) $(LIBOBJ)

.c.o:
	@echo lib Makefile - compiling $<
	@$(CC) $(CFLAGS) -c $< -o $@


#-------------------------------------------------------


# # source files.
# SRC =  	file1.cpp file2.cpp file3.cpp
 
# OBJ = $(SRC:.cpp=.o)
 
# OUT = ../libutils.a
 
# # include directories
# INCLUDES = -I. -I../include/ -I/usr/local/include
 
# # C++ compiler flags (-g -O2 -Wall)
# CCFLAGS = -g
 
# # compiler
# CCC = g++
 
# # library paths
# LIBS = -L../ -L/usr/local/lib -lm
 
# # compile flags
# LDFLAGS = -g
 
# .SUFFIXES: .cpp
 
# default: dep $(OUT)
 
# .cpp.o:
# 	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@
 
# $(OUT): $(OBJ)
# 	ar rcs $(OUT) $(OBJ)
 
# depend: dep
 
# dep:
# 	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)
 
# clean:
# 	rm -f $(OBJ) $(OUT) Makefile.bak

