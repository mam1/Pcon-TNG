#############################################################################
#	basic mint sql Pcon build												#
############################################################################# 

program_NAME := test-SQL

# scan directory 
program_C_SRCS := $(wildcard *.c)
program_CXX_SRCS := $(wildcard *.cpp)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS)

# set directories
program_INCLUDE_DIRS := /usr/lib/mysql/mysql-connector-c-6.1.9-linux-glibc2.5-x86_64/include 	#/home/mam1/Git/Pcon-TNG/BBB/include 
program_LIBRARY_DIRS := /usr/lib/mysql/mysql-connector-c-6.1.9-linux-glibc2.5-x86_64/lib 		
program_LIBRARIES :=  :libmysqlclient.so.18	

# set parameters
CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))
CFLAGS  += -Wall
CXXFLAGS += $(CFLAGS)
LDFLAGS += $(CFLAGS) 															

.PHONY: all clean distclean

all: $(program_NAME)

$(program_NAME): $(program_OBJS)
	gcc $(program_OBJS) -o $(program_NAME) $(CFLAGS) $(CPPFLAGS)
	
clean:
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS) 

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

distclean: clean