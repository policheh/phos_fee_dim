#!/bin/bash
#-----------------------------------------------------------------------------
# This script checks if 14 PHOS FEE DIM servers are running and
# if not, start them 
# Yuri Kharlov. 04.04.2016
#-----------------------------------------------------------------------------

LOGDIR=$HOME/var/log
if [ ! -d $LOGDIR ]; then
    echo Directory $LOGDIR does not exist. Create it.
    mkdir -pv $LOGDIR
fi
FEEDIR=$HOME/PHOS_FEE_DIM
cd $FEEDIR
export DIM_DNS_NODE=aliphson001

for SRU in `seq 1 14`; do
    IP=10.160.136.$((16+SRU-1))
    PID=`pgrep -f "PhsFeeDim $SRU $IP"`
    if [ ! $PID ]; then
	echo Process \"PhsFeeDim $SRU \" does not run, restart it
	LOGFILE=$LOGDIR/PhsFeeDim_$SRU.log
	rm -rf $LOGFILE
##	./PhsFeeDim $SRU $IP >& $LOGFILE &
	./PhsFeeDim $SRU $IP >& /dev/null &
    else
	echo Process \"PhsFeeDim $SRU \" runs as $PID
    fi
done
exit
