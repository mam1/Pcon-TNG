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

lib_SRCS := $(wildcard *.sh)


.PHONY: all clean distclean

all: 
# scan directory 
	for lib_SRCS %%f in (*.sh); \
	do \
	scripts := $(scripts) %%~nf; \
	done
# move the scripts to cgi directory on the bone
	rsync -avz $(program_DIR)/$(scripts) root@192.168.254.34:/usr/local/sbin/$(scripts)
	ssh root@192.168.254.34 'chown root /usr/local/sbin/$(scripts)'		# change owner
	ssh root@192.168.254.34 'chmod 4755 /usr/local/sbin/$(scripts)'		# open access
os:
	@echo "****************"
	@echo $(UNAME_S)






