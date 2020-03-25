EXP="test"
j=1
while [ $j -le $2 ] ; do
	      echo "Stopping attack on attacker$j "
	      ssh -o StrictHostKeyChecking=no  attacker$j.$EXP.leader "sudo pkill -9 python" &
	      j=$(($j+1))

done
sleep $INT
echo "Stopping legitimate traffic"
j=0
ssh -o StrictHostKeyChecking=no  attacker$j.$EXP.leader "sudo killall python3; sudo pkill -9 python" &
