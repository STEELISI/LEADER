# LEADER: Low-Rate Denial of Service Defense

## About
Leader is a novel application-agnostic and  attack-agnostic defense against exploit-based Denial-of-Service Attacks (exDoS)

[Read more](https://steel.isi.edu/projects/Leader/)

## Installation
The installation uses CMake and assumes that you have the following libraries available:
* Boost System - `libboost-system-dev` in Debian
* Intel TBB - `libtbb-dev` in Debian
* libpython3 - `python3-dev` in Debian

We assume that the latest libraries are being used.
 
In order to build LEADER, run the following:
```bash
mkdir build && cd build
cmake ..
make
```

## HOW TO RUN
./build/leader elliptic_envelope.mlmodel normalization.pkl standardization.pkl

## Members and Collaborators
PI: Jelena Mirkovic and Christophe Hauser
* Genevieve Bartlett
* Rajat Tandon
* Haoda Wang
* Nicolaas Weideman
* Shushan Arakelyan



## Links for Installing PHP 5.3:
https://askubuntu.com/questions/1052746/can-i-install-php-5-3-5-on-ubuntu-server-18-04-lts
./configure --with-apxs2
LoadModule php5_module /usr/lib/apache2/modules/libphp5.so
https://www.php.net/manual/en/install.unix.apache2.php

