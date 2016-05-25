#############################################################################
#	move scripts to the bone and fix ownership and permissions																#
############################################################################# 

# scan directory 
scripts := $(wildcard *.sh)


.PHONY: all clean distclean

all: 

# move the scripts to cgi directory on the bone
	rsync -avz /Users/mam1/Git/Pcon-TNG/script_lib/$(scripts) root@192.168.254.34:/usr/local/sbin/$(scripts)
	ssh root@192.168.254.34 'chown root /usr/local/sbin/$(scripts)'		# change owner
	ssh root@192.168.254.34 'chmod 4755 /usr/local/sbin/$(scripts)'		# open access





