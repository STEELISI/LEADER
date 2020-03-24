EXP="test"
DURATION=60
INT=10
j=0

echo "Starting leg traffic"
ssh -o StrictHostKeyChecking=no  attacker$j.$EXP.leader "sudo python3 /users/rajat19/frade/traffic/smart_attacker/legitimate.py -s $1 --sessions 100 --logs /proj/Leader/imgur-new-sorted-only-200s.log --interface enp6s0f2 &" &

echo "Sleeping"
sleep $INT
j=1

while [ $j -le $2 ] ; do
	      echo "Starting attack on attacker$j $2 "
	      ssh -o StrictHostKeyChecking=no  attacker$j.$EXP.leader "cd /users/rajat19/PySlowLoris/ ; sudo python src/main.py $1" &
              j=$(($j+1))
done
sleep $DURATION
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
