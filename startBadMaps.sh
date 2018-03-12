#!/bin/bash

FEEDIR=$HOME/PHOS_FEE_DIM
cd $FEEDIR

export DIM_DNS_NODE=aldaqecs.cern.ch
export LD_LIBRARY_PATH=/usr/lib64/root

PID=`pgrep -f buildBadMaps`

if [ ! $PID ]; then
    echo Process \"buildBadMaps\" does not run, restart it
    rm -rf buildBadMaps.log
    nohup ./buildBadMaps >& buildBadMaps.log &
else
    echo Process \"buildBadMaps\" runs as $PID
fi
exit
