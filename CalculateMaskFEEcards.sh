#!/bin/bash

# Standard configuration on P2,
# when all 28 cards in both branches are switched ON:
# CalculateMaskFEEcards.sh 0 15 16 17 18 19 20 35 36 37 38 39
# 0x7ffe 0x7ffe


lowerpower=0xfffff # everything is switched ON by default
upperpower=0xfffff # everything is switched ON by default

lowermasked=0x0 # nothing in readout is masked by default
uppermasked=0x0 # nothing in readout is masked by default

for i; do
    echo Switch off DTC $i

    if [ $i -gt 19 ]
    then
        targetmasklower="0x0"
        targetmaskupper=$((2 ** ($i - 20)))
	

    else
        targetmasklower=$((2 ** $i))
        targetmaskupper="0x0"

    fi

lowerpower=$(($lowerpower ^ $targetmasklower))
upperpower=$(($upperpower ^ $targetmaskupper))

done

lowerpowernew=$(printf "0x%x" $lowerpower)
upperpowernew=$(printf "0x%x" $upperpower)

lowread=$(printf "0x%x" $(($lowerpowernew ^ 0xfffff)))
uppread=$(printf "0x%x" $(($upperpowernew ^ 0xfffff)))

echo maskL: $lowerpowernew maskH: $upperpowernew
echo Readout maskL: $lowread maskH: $uppread
