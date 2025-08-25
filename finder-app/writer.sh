#!/bin/bash
# Assignment 1 script writer.sh
# Author: Tanvi sharma

writefile = $1
writestr = $2

if [ $# -lt 2 ]
then
	echo "Not enough arguments"
	exit 1
else
	cat > $writefile
	
	if [! $? eq 0]
	then
		echo "Error creating file"
	else
		cat >> $writestr
		echo "File created successfully with data ${writestr}"
		exit 0
	fi
fi
