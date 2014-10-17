#!/bin/bash
#set -x

#before running this command, use regdir.sh to process the images in a directory, then load the processed images into imagej and threshold them. finally, ensure that 'set measurements' is set to display center of mass, area, and stack position to zero decimal places, and run analyze particles with pixel size 50-infinity and circularito 0.2 to 1. Save the results in the directory where the processed images are

#to call clustertracker with the right arguements and such

OUTDIR=/Users/Shared/Results
CLUSTDIST=55
MINCUBES=3
TRACKER=~/bin/clustertracker

#find dimensions file produced by regintensity
NEWDIMS=$(ls | grep -i dimensions | head -n 1)  
while [ "$NEWDIMS" == "" ]; do
    echo "Dimensions file not found. please enter path to dimensions file"
    read dpath
    if [ -f $dpath ]; then
	mv $dpath dimensions_processed.txt
    else
	echo "$dpath does not exist"
    fi
    NEWDIMS=$(ls | grep -i dimensions | head -n 1)
done

NEWH=$(cat $NEWDIMS | tr ' ' '\n' | head -n 1)
NEWW=$(cat $NEWDIMS | tr ' ' '\n' | tail -n 1)

RESULTS=Results.txt
while [ ! -f $RESULTS ]; do
    echo "Results file not found. please enter path to dimensions file"
    read dpath
    if [ -f $dpath ]; then
	mv $dpath Results.txt
    else
	echo "$dpath does not exist"
    fi
done

SLICES=$(tail -n 1 "$RESULTS" | tr "\t" ":" | cut -d ":" -f 5 ) 

#name the outfile
OUTFILE="$(ls | grep -i jpg | head -n 1 | sed 's/-[0-9]*[0-9]_processed.*/.csv/')"
echo "Outfile will be named $OUTFILE in $OUTDIR"

$TRACKER $RESULTS $SLICES $NEWH $NEWW $CLUSTDIST $MINCUBES > $OUTDIR/$OUTFILE