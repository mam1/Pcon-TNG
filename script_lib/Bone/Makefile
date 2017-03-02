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
	target_dir := 
endif

.PHONY: all clean distclean

all: 
	@echo $(lib_SRCS)
# move the scripts to cgi directory on the bone
	rsync -avz $(program_DIR)/bone/ root@192.168.254.34:/usr/local/sbin/

	ssh root@192.168.254.34 'chown root /usr/local/sbin/*'		# change owner
	ssh root@192.168.254.34 'chmod 4755 /usr/local/sbin/*'		# open access

# move the scripts to development machines
ifeq ($(UNAME_S),Darwin)
	rsync -avz $(program_DIR)/bone/ root@192.168.254.34:/usr/local/sbin/


# fix permissions for scripts run on development machines

os:
	@echo "****************"
	@echo $(UNAME_S)






