#!/bin/bash
# start up script for 16 channel controller

echo -e \\033c		# clear the terminal screen
stty cooked			# switch to buffered iput
stty echo			# turn on terminal echo

echo "************************
  starting Pcon system
************************"
echo "terminal io reinitialized"
echo "checking for active processes"

# echo $(pidof /usr/local/sbin/Dcon)
	plist=$(pidof /usr/local/sbin/Dcon)
# echo $plist
if [ "$plist" != "" ]; then
		hlist="there is an instance of Dcon active pid" 
		hlist=$hlist$plist
		echo $hlist
		echo "kill instance <y><n> ? "
		read answr
		if["$answr" == "y"]; then
			echo "killing it"
			kill $plist
		elif then;
			echo -n "process "
			echo -n $hlist
			echo "left running"
		fi
	elif
		echo "there are no instances of Dcon active"
fi

plist=$(pidof /usr/local/sbin/Pcon)
if [ "$plist" != "" ]; then
		hlist="there is an instance of Pcon active pid " 
		hlist=$hlist$plist
		echo $hlist
		echo "killing it"
		kill $plist
elif
	echo "there are no instances of Pcon active"
fi
echo "starting daemon"
Dcon > /home/Pcon-data/Dcon.log &
echo "Dcon started"
sleep 10
Pcon 

exit 0