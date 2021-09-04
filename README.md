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


=======================================================================
USC-RL v1.0
The Software is made available for academic or non-commercial purposes only. The license is for
a copy of the program for an unlimited term. Individuals requesting a license for commercial use
must pay for a commercial license.
USC Stevens Institute for Innovation
University of Southern California
1150 S. Olive Street, Suite 2300
Los Angeles, CA 90115, USA
ATTN: Accounting
DISCLAIMER. USC MAKES NO EXPRESS OR IMPLIED WARRANTIES, EITHER IN FACT OR BY
OPERATION OF LAW, BY STATUTE OR OTHERWISE, AND USC SPECIFICALLY AND EXPRESSLY
DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A
PARTICULAR PURPOSE, VALIDITY OF THE SOFTWARE OR ANY OTHER INTELLECTUAL PROPERTY
RIGHTS OR NON-INFRINGEMENT OF THE INTELLECTUAL PROPERTY OR OTHER RIGHTS OF ANY
THIRD PARTY. SOFTWARE IS MADE AVAILABLE AS-IS.
LIMITATION OF LIABILITY. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT WILL
USC BE LIABLE TO ANY USER OF THIS CODE FOR ANY INCIDENTAL, CONSEQUENTIAL, EXEMPLARY
OR PUNITIVE DAMAGES OF ANY KIND, LOST GOODWILL, LOST PROFITS, LOST BUSINESS AND/OR
ANY INDIRECT ECONOMIC DAMAGES WHATSOEVER, REGARDLESS OF WHETHER SUCH DAMAGES
ARISE FROM CLAIMS BASED UPON CONTRACT, NEGLIGENCE, TORT (INCLUDING STRICT LIABILITY
OR OTHER LEGAL THEORY), A BREACH OF ANY WARRANTY OR TERM OF THIS AGREEMENT, AND
REGARDLESS OF WHETHER USC WAS ADVISED OR HAD REASON TO KNOW OF THE POSSIBILITY OF
INCURRING SUCH DAMAGES IN ADVANCE.
For commercial license pricing and annual commercial update and support pricing, please
contact:
<Licensing Associate Name>
USC Stevens Institute for Innovation
University of Southern California
1150 S. Olive Street, Suite 2300
Los Angeles, CA 90015, USA
Tel: <Licensing Associate phone number>
Fax: +1 213-821-5001
Email: <Licensing Associate Email> and cc to: accounting@stevens.usc.edu
