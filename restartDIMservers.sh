#!/bin/bash
#-----------------------------------------------------------------------------
# This script kills and restars one PHOS FEE DIM server
# Yuri Kharlov. 20.04.2017
#-----------------------------------------------------------------------------

if [ ! $1 ]; then
    echo "Usage: $0 <SRU #>"
    echo "restart FEE DIM server fora given SRU (from 1 to 14)" 
    exit 1
fi

if [ $1 -lt 1 ] || [ $1 -gt 14 ]; then
    echo "Wrong SRU number $1, must be from 1 to 14"
    exit 1
fi

FEEDIR=$HOME/PHOS_FEE_DIM
cd $FEEDIR
export DIM_DNS_NODE=aliphson001

SRU=$1
IP=10.160.136.$((16+SRU-1))
PID=`pgrep -f "PhsFeeDim $SRU $IP"`
if [ $PID ]; then
    echo "Process \"PhsFeeDim $SRU\" runs with PID $PID, kill and restart it"
    kill -9 $PID
    ./PhsFeeDim $SRU $IP >& /dev/null &
    PID=`pgrep -f "PhsFeeDim $SRU $IP"`
    echo "Process \"PhsFeeDim $SRU\" restarted with PID $PID"
else
    echo "Process \"PhsFeeDim $SRU\" does not run, start it"
    ./PhsFeeDim $SRU $IP >& /dev/null &
fi

exit 0
