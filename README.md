## Overview
IEC61850_CyberSecurity Project - Machine Learning based Intrusion Detection System for IEC61850-MMS Profile

Contents:
- Server_APP - High-interaction Honeypot 
- Machine Learning based detection tool 
- AttackClient 
- Jupyter Notebook 

Tested on Debian Linux - Ubuntu and Rpi3  

Prerequisities:
- apt-get update
- apt-get install libsqlite3-dev build-essential iptables-persistent git tcpdump python3-pip doxygen cmake swig
- apt-get install autoconf automake libtool pkg-config libpcap-dev python3-pip python3-dev python3-numpy python3-scipy
- pip3 install pandas sklearn sklearn-deltatfidf cffi argparse

Deploymet (:Copy-Paste:)
- cd opt/
- git clone https://github.com/ArtWachowski/iec_61850.git;
- cd iec_61850;
- cmake -DBUILD_PYTHON_BINDINGS=ON -DBUILD_EXAMPLES=ON -DDEBUG=OFF -DCONFIG_ACTIVATE_TCP_KEEPALIVE=OFF . 
- make 
- make install
- cp examples/server_app/server_app /usr/bin/;
- server_app &
- LD_LIBRARY_PATH=/usr/local/lib
- export LD_LIBRARY_PATH
- cd CC_SNIFER
- gcc cc_sniffer.c -o cc_sniffer -I /usr/include/python3.8 -lpython3.8 -lpcap
- cp iec61850/cc_sniffer /usr/bin/;
- echo 'Finished';

Run IEC61850_pot:
server_app

Run cc_snifer tool:
cc_sniffer

Run attackclient.py from iec61850 directory. 

Jupyter - navigate to directory and run "jupyter notebook" command

libiec61850 is an open-source (GPLv3) implementation of an IEC 61850 client and server library implementing the protocols MMS, GOOSE and SV, credit goes to MZ_Autoimation.
