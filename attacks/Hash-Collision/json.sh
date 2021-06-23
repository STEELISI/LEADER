#!/bin/bash
input="collision_keys.txt"
inp="/proj/Leader/attacks/PySlowLoris/attackerIPs/10000/IPs_for_att_1"
i=0
st="{"
while IFS= read -r line
do
    i=$((i+1))
    if [ "$i" -lt "50000" ]; then
    st="${st} \"$line\":\"key$i\","

    fi
    if [ "$i" -gt "50000" ]; then
            st="${st} \"$line\":\"key$i\" }"
            break 
    fi    
done < "$input"
          
echo $st
