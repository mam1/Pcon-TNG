#!/bin/bash
# push the routers external IP address to dropbox

APP="push_ip"
echo -e "\n\r" $APP " 0.0.0\n\r"
CURRENT_OS=$(uname)
echo -n "operating system is "
echo $CURRENT_OS

if [ $CURRENT_OS == "Linux" ]
then
	echo -e "\rappending active sensor log to master log\r"

else
	echo -e "\r" $APP " can not be run from OS X\n\r"
fi
