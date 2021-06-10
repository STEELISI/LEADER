sudo apt-get update
sudo apt-get install python3-pip
sudo apparmor_parser -R /etc/apparmor.d/usr.sbin.tcpdump 
sudo pip3 install netifaces
sudo pip3 install python-dateutil
sudo pip3 install shove
#cd ~/frade/traffic/smart_attacker/
#sudo bash install
cd ~/frade//experiments/setup/ip_addrs/; sudo perl assign 10.1.x.x
#python3 legitimate.py -s 10.1.1.2 --sessions 100 --logs /proj/Leader/access.log --interface enp6s0f1
