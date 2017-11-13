#!/bin/bash


gpscount=`egrep '^,,,' $1 |wc -l`
cancount=`egrep ',,,$' $1 |wc -l`

echo "GPS messages ${gpscount}"
echo "CAN messages ${cancount}"
