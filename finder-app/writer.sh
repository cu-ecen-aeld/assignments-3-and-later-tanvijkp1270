#!/bin/bash
# Assignment 1 script writer.sh
# Author: Tanvi sharma

writefile=$1
writestr=$2
directoryname=$(dirname "$writefile")

if [ $# -lt 2 ]
then
	echo "Not enough arguments"
	exit 1
else
	mkdir -p $directoryname
	echo "$writestr" > $writefile
	
	ls -l $writefile
	
	if [ $? -ne 0 ]
	then
		echo "Error creating file"
		exit 1
	else
		echo "File created successfully with data $writestr"
		exit 0
	fi
fi
