#!/bin/bash
# run as root

# Install dependencies
apt-key adv --keyserver keyserver.ubuntu.com --recv-keys C8CAB6595FDFF622
codename=$(lsb_release -c | awk  '{print $2}')
sudo tee /etc/apt/sources.list.d/ddebs.list << EOF
deb http://ddebs.ubuntu.com/ ${codename}      main restricted universe multiverse
deb http://ddebs.ubuntu.com/ ${codename}-updates  main restricted universe multiverse
deb http://ddebs.ubuntu.com/ ${codename}-proposed main restricted universe multiverse
EOF

sudo apt update
sudo apt install -y linux-image-$(uname -r)-dbgsym libdw-dev libboost-thread-dev libboost-system-dev libboost-timer-dev libboost-filesystem-dev libboost-atomic-dev libboost-date-time-dev
sudo apt clean

# Build Dyninst
#wget https://github.com/dyninst/dyninst/archive/v9.3.2.tar.gz
#tar xvf v9.3.2.tar.gz && rm -rf v9.3.2.tar.gz
#cd dyninst-9.3.2/
#cmake .\
#make && sudo make install
#cd ../ && sudo rm -rf dyninst-9.3.2/

# Patch and build Systemtap
wget https://sourceware.org/systemtap/ftp/releases/systemtap-4.0.tar.gz
tar xvf systemtap-4.0.tar.gz && rm -rf systemtap-4.0.tar.gz
cd systemtap-4.0
./configure && make && sudo make install
cd ../ && sudo rm -rf systemtap-4.0

# Generate kernel function map
cp /proc/kallsyms /boot/System.map-`uname -r`

cd /mnt
# Get and install CMake
wget https://github.com/Kitware/CMake/releases/download/v3.17.0-rc1/cmake-3.17.0-rc1-Linux-x86_64.tar.gz
tar xvf cmake-3.17.0-rc1-Linux-x86_64.tar.gz
cd cmake-3.17.0-rc1-Linux-x86_64/
rsync -K -a . /usr/local

cd /mnt
# Get and install TBB
wget https://github.com/intel/tbb/releases/download/v2020.1/tbb-2020.1-lin.tgz
tar xvf tbb-2020.1-lin.tgz
rm -rf pstl && cd tbb
cp -r bin/* /usr/local/bin/
cp -r include/* /usr/local/include/
cp -r lib/* /usr/local/lib/
cp ./cmake/*.cmake /usr/local/share/cmake-3.17/Modules/
cp ./cmake/templates/* /usr/local/share/cmake-3.17/Templates/
