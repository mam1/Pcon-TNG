#!/bin/bash
#
#	set-image

if [ $# != 2 ]
	then
		printf("usage: set-image <source location> <target location>\n")
		exit 1
	fi


printf("copy $1  to $2")

