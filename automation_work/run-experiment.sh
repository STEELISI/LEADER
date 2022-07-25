#!/usr/local/bin//bash

SERVER="server"
EXP="exp"
PATH_TO_LEADER_FOLDER="/mnt/local/LEADER/"
INT=60
DURATION=600
LOG="/mnt/LOG_ONLY_TIME.txt"


#Starting Blacklisting Module and LEADER

ssh  -o StrictHostKeyChecking=no $SERVER.$EXP.leader "cd $PATH_TO_LEADER_FOLDER/mitigation; sudo bash clear_blacklisting.sh; sudo start_blacklisting.sh &; cd $PATH_TO_LEADER_FOLDER; ./build/leader elliptic_envelope.mlmodel normalization.pkl standardization.pkl >> $LOG &" &

#Starting Legitimate Traffic
echo "Starting leg traffic"
ssh -o StrictHostKeyChecking=no  attacker0.$EXP.leader "cd ~/frade/traffic/smart_attacker/; sudo python3 legitimate.py -s 10.1.1.2 --sessions 300 --logs /proj/Leader/access.log --interface enp4s0f1 &" &

sleep $INT

#Starting Attack Traffic
echo "Starting attack on Attacker 1"
ssh -o StrictHostKeyChecking=no  attacker1.$EXP.leader "cd /proj/Leader/Slowloris/with-multiplexing/PySlowLoris; sudo python src/main.py $SERVER -cf /mnt/100.txt &; sudo python src/main.py $SERVER -cf /mnt/100.txt &; sudo python src/main.py $SERVER -cf /mnt/100.txt &;" & 

#ssh -o StrictHostKeyChecking=no  attacker$j.$EXP.frade "cd $path_to_execute_from/experiments/run/;sudo httperf --server=$1 --uri=$homepage --num-conns=1200000 --rate=1000 --timeout=1 --multiaddress=addresses$j.txt --hog" &

sleep $DURATION

ssh -o StrictHostKeyChecking=no attacker1.$EXP.leader "sudo killall -9 python &" &

sleep $INT

ssh -o StrictHostKeyChecking=no attacker0.$EXP.leader "sudo killall -9 python3 &" &
ssh -o StrictHostKeyChecking=no $SERVER.$EXP.leader "sudo killall -9 leader &; sudo killall -9 python &" &



