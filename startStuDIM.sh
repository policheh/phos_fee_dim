#!/bin/bash
#-----------------------------------------------------------------------------
# This script checks if STU DIM server is running and
# if not, start it 
# Boris Polishchuk. 01.06.2017
#-----------------------------------------------------------------------------

STUDIR=$HOME/PHOS_STU_DIM
cd $STUDIR

export DIM_DNS_NODE=aliphson001
STUPID=`pgrep -f PhsStuDim`

if [ ! $STUPID ]; then
    echo Process \"PhsStuDim\" does not run, restart it
    ./PhsStuDim >& /dev/null &
else
    echo Process \"PhsStuDim\" runs as $STUPID
fi

exit

