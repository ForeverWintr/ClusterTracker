#!/bin/bash
#set -x

#run registration and intensity equalization on the contents of a directory
#By Tom Rutherford

#variables
BG=background.jpg
OUT=./output
TAIL=_processed.jpg #this is the tail that is added by regintensity.cc
#if you want to run regdir.sh from a different directory, make sure
#the below line has the complete path to the regintensity binary.
REGINTENSITY=/Users/Shared/tTestrepo/clustertracker/regintensity/regintensity 

#remove spaces from filenames
prevIFS=$IFS
IFS='
'
for name in $(ls); do
    newname=$(echo "$name" | tr ' ' '-')
    if [ ! "$name" = "$newname" ]; then
	mv "$name" "$newname"
	echo "$name has been renamed $newname" 
    fi
done 
IFS=$prevIFS

#get corner coordinates
if [ ! -e coordinates.txt ]; then
    echo "Please enter coordinates for the four corners of the arena
in the order top left top right bottom left bottom right, and format:
height width. Note that the origin, (0, 0) is in the top left corner
of the image."
    read coordinates
    
    echo "$coordinates" > coordinates.txt 
    
else
    coordinates=$(cat coordinates.txt)
fi

#ensure we know which image is the background image
if [ ! -e "$BG" ]; then # (if there isn't already an image called background)
    background=$(ls | grep jpg | head -n 1)
    echo "is $background the background image? (y/n)"
    read prompt
    if [ "$prompt" != "y" -a "$prompt" != "Y" ]; then 
	echo "Please enter the name of the background image"
	read background
    fi
    while [ ! -e "$background" ]; do
	echo "ERROR: $background does not exist, please retry:"
	read background
    done
    mv $background "$BG"
fi

#set up a directory for storing processed images, and make sure it is empty
if [ -d "$OUT" ]; then # (if output directory exists)
    if [ $(ls -A "$OUT" | wc -l) -ne 0 ]; then
	echo "$OUT (the output directory) already exists and is not empty. Overwrite (y/n)?"
	read yn
	if [ "$yn" = "y" -o "$yn" = "Y" ]; then
	    rm -rf "$OUT"
	    mkdir "$OUT"
	else
	    echo "Ok, exiting."
	    exit 2
	fi
    fi
fi
mkdir -p "$OUT"

JPGS="background.jpg $(ls | grep jpg | grep -v background) done"

echo "$JPGS" | $REGINTENSITY $coordinates

mv -f *"$TAIL"* $OUT 