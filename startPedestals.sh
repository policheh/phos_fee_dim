#!/bin/bash

FEEDIR=$HOME/PHOS_FEE_DIM
cd $FEEDIR

export DIM_DNS_NODE=aldaqecs.cern.ch
export LD_LIBRARY_PATH=/usr/lib64/root

PID=`pgrep -f buildPedestals`

if [ ! $PID ]; then
    echo Process \"buildPedestals\" does not run, restart it
#    ./buildPedestals >& /dev/null &
    rm -rf buildPedestals.log
    nohup ./buildPedestals >& buildPedestals.log &
else
    echo Process \"buildPedestals\" runs as $PID 
fi
exit
