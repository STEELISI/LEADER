# LEADER: Low-Rate Denial of Service Defense

## About
LEADER leverages a novel combination of runtime monitoring and offline binary program analysis to protect a deploying server against different variants of Low-Rate DDoS attacks.

[Read more](https://steel.isi.edu/Projects/ddosdefense/)

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

## Members and Collaborators
* Haoda Wang
* Christophe Hauser
* Jelena Mirkovic
* Nicolaas Weideman
* Rajat Tandon
