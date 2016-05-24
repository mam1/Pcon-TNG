# move the executable to cgi directory on the bone
	rsync -avz /Users/mam1/Git/Pcon-TNG/BBB/CGI/Scon root@192.168.254.34:/usr/lib/cgi-bin
	ssh root@192.168.254.34 'chown root /usr/lib/cgi-bin/Scon'		# change owner
	ssh root@192.168.254.34 'chmod 4755 /usr/lib/cgi-bin/Scon'		# open access