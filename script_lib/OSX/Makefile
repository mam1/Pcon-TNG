#############################################################################
#	move scripts to /usr/local/myscripts and fix ownership and permissions																#
############################################################################# 

# get the development machine OS
UNAME_S := $(shell uname -s)

# set directories for scripts that need to be moved to the bone
ifeq ($(UNAME_S),Darwin)
	program_DIR := /Users/mam1/Git/Pcon-TNG/script_lib
	target_dir := /usr/local/myscripts
else
	program_DIR := /home/mam1/Git/Pcon-TNG/script_lib
	target_dir := /usr/local/myscripts
endif

.PHONY: all clean distclean

all: 
	@echo $(lib_SRCS)

# move the scripts to script library on the mac
ifeq ($(UNAME_S),Darwin)
	rsync -avz $(program_DIR)/OSX/ /usr/local/myscripts/
else
	rsync -avz $(program_DIR)/Mint/ /usr/local/myscripts/
endif

	chown mam1 /usr/local/myscripts/*
	chmod 4755 /usr/local/myscripts/*

os:
	@echo "****************"
	@echo $(UNAME_S)






