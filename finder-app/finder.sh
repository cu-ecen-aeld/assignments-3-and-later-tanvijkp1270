#!/bin/bash
# Assignment 1 script finder.sh
# Author: Tanvi sharma

filesdir=$1
searchstr=$2

if [ $# -lt 2 ]
then
	echo "Not enough arguments"
	exit 1
elif [ ! -d $filesdir ]
then 
	echo "Directory not found, check and enter correct path"
	exit 1
else
	NUMFILES=$(ls $filesdir | wc -l)
	MATCHINGLINES=$(grep -rc "$searchstr" "$filesdir" | wc -l)
	echo "The number of files are $NUMFILES and the number of matching lines are $MATCHINGLINES"
	exit 0
fi

