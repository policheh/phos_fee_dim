#!/bin/bash
#-----------------------------------------------------------------
# This scripts (re)configures and apply APD bias
# in 14 parallel subprocesses.
# Boris Polishchuk, 19.06.2015
#-----------------------------------------------------------------

if [ ! $1 ]
then
    echo "Usage: $0 <run type>"
    echo "       to create common paramter for SRU, FEE and ALTRO."
    echo "       run type can be: UNDEFINED,PHYS,LED,PED,STANDALONE."
    exit
fi

runtype=$1

./ConfigureAllFEE 1 $runtype &
./ConfigureAllFEE 2 $runtype &
./ConfigureAllFEE 3 $runtype &
./ConfigureAllFEE 4 $runtype &
./ConfigureAllFEE 5 $runtype &
./ConfigureAllFEE 6 $runtype &
./ConfigureAllFEE 7 $runtype &
./ConfigureAllFEE 8 $runtype &
./ConfigureAllFEE 9 $runtype &
./ConfigureAllFEE 10 $runtype &
./ConfigureAllFEE 11 $runtype &
./ConfigureAllFEE 12 $runtype &
./ConfigureAllFEE 13 $runtype &
./ConfigureAllFEE 14 $runtype &



