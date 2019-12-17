bash $path_to_execute_from/experiments/setup/setuproute $1
sudo apt-get update
sudo apt-get install -y libboost-all-dev ipset
make clean; make
sudo apparmor_parser -R /etc/apparmor.d/usr.sbin.tcpdump
sudo mkdir /zfs
sudo mkdir /zfs/LEADER
sudo mount -t nfs -o tcp,vers=3 zfs:/zfs/LEADER /zfs/LEADER
