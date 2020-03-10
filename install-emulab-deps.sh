#!/bin/bash
# run as root
cd /mnt || exit
# Install dependencies
apt-key adv --keyserver keyserver.ubuntu.com --recv-keys C8CAB6595FDFF622
codename=$(lsb_release -c | awk  '{print $2}')
sudo tee /etc/apt/sources.list.d/ddebs.list << EOF
deb http://ddebs.ubuntu.com/ ${codename}      main restricted universe multiverse
deb http://ddebs.ubuntu.com/ ${codename}-updates  main restricted universe multiverse
deb http://ddebs.ubuntu.com/ ${codename}-proposed main restricted universe multiverse
EOF

sudo apt update
sudo apt install -y "linux-image-$(uname -r)-dbgsym libdw-dev libboost-all-dev python3-dev python3-pip python-pip python-dev"
sudo apt clean
sudo pip3 install scikit-learn==0.22.2 numpy pandas

# Patch and build Systemtap
git clone git://sourceware.org/git/systemtap.git
cd systemtap || exit
git checkout release-4.2
./configure && make && sudo make install
cd ../ && sudo rm -rf systemtap-4.0

# Generate kernel function map
cp /proc/kallsyms "/boot/System.map-$(uname -r)"

cd /mnt || exit
# Get and install CMake
wget https://github.com/Kitware/CMake/releases/download/v3.17.0-rc1/cmake-3.17.0-rc1-Linux-x86_64.tar.gz
tar xvf cmake-3.17.0-rc1-Linux-x86_64.tar.gz
cd cmake-3.17.0-rc1-Linux-x86_64/ || exit
rsync -K -a . /usr/local

cd /mnt || exit
# Get and install TBB
wget https://github.com/intel/tbb/releases/download/v2020.1/tbb-2020.1-lin.tgz
tar xvf tbb-2020.1-lin.tgz
rm -rf pstl
cd tbb || exit
cp -r bin/* /usr/local/bin/
cp -r include/* /usr/local/include/
cp -r lib/intel64/gcc4.8/* /usr/local/lib/
cp ./cmake/*.cmake /usr/local/share/cmake-3.17/Modules/
cp ./cmake/templates/* /usr/local/share/cmake-3.17/Templates/

# Add /usr/share/lib to ld
ldconfig /usr/local/lib
echo '/usr/local/lib' > /etc/ld.so.conf.d/tbb.conf
