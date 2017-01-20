#############################################################################
#	move scripts to the bone and fix ownership and permissions																#
############################################################################# 

# get the development machine OS
UNAME_S := $(shell uname -s)

# set directories
ifeq ($(UNAME_S),Darwin)
	program_DIR := /Users/mam1/Git/Pcon-TNG/script_lib
else
	program_DIR := /home/mam1/Git/Pcon-TNG/script_lib
endif
lib_SRCS := $(wildcard *)


.PHONY: all clean distclean

all: 
	@echo $(lib_SRCS)
# move the scripts to cgi directory on the bone
	rsync -avz $(program_DIR)/ root@192.168.254.34:/usr/local/sbin/

	ssh root@192.168.254.34 'chown root /usr/local/sbin/*'		# change owner
	ssh root@192.168.254.34 'chmod 4755 /usr/local/sbin/*'		# open access

os:
	@echo "****************"
	@echo $(UNAME_S)






