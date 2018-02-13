#!/bin/bash

set_hv_in_sru()
{
# Usage: set_hv_in_sru 2 0
# will set APD biases in M2-0

    mod=$1
    partition=$2
    
    SRU=$(echo "4*($mod-1)+$partition-1"|bc)
    rcu=$partition
   
    echo "" 
    echo Setting APD biases for M$mod-$partition, SRU $SRU

    dtc=1 branch=0
    while [ $dtc -lt 15 ]
    do
	file=$(printf "APDsettings/set_bias_dtc_Module%s_RCU%s_branch%s_card%s.txt" "$mod" "$rcu" "$branch" "$dtc")
	./setHV $file $SRU $dtc
	dtc=`expr $dtc + 1`
    done

    dtc=21 card=1 branch=1
    while [ $dtc -lt 35 ]
    do
	file=$(printf "APDsettings/set_bias_dtc_Module%s_RCU%s_branch%s_card%s.txt" "$mod" "$rcu" "$branch" "$card")
	./setHV $file $SRU $dtc
	dtc=`expr $dtc + 1`
	card=`expr $card + 1`
    done
    
}

# Module 1
for part in 2 3
do
    set_hv_in_sru 1 $part
done

# Modules 2-4
for mod in 2 3 4 
do
    for part in 0 1 2 3
    do
	set_hv_in_sru  $mod $part
    done
done
