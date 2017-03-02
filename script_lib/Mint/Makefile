#############################################################################
#	move scripts to the bone and fix ownership and permissions																#
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
all: 
	@echo $(lib_SRCS)

# move the scripts to script library on the mac
	rsync -avz $(program_DIR)/Mint/ /usr/local/myscripts/

	$(shell chown mam1 /usr/local/myscripts/*)		# change owner
	$(shell chmod 4755 /usr/local/myscripts/*)		# open access

os:
	@echo "****************"
	@echo $(UNAME_S)






