# LEADER: Low-Rate Denial of Service Defense

## About
LEADER leverages a novel combination of runtime monitoring and offline binary program analysis to protect a deploying server against different variants of Low-Rate DDoS attacks.

[Read more](https://steel.isi.edu/Projects/ddosdefense/)

## Installation
The installation uses CMake and assumes that you have the following libraries available:
* Boost System - `libboost-system-dev` in Debian/Ubuntu
* libbobcat - `libbobcat-dev` in Debian/Ubuntu
* libpython3 - `python3-dev` in Debian/Ubuntu

In order to build LEADER, run the following:
```bash
mkdir build && cd build
cmake ..
make
```

## Code Walkthrough
LEADER is divided into three libraries: the connection life stage builder, the scorer, and the mitigator.

## Members and Collaborators
* Haoda Wang
* Christophe Hauser
* Jelena Mirkovic
* Nicolaas Weideman
* Rajat Tandon
