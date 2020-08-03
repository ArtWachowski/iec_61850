## Overview
IEC61850_CyberSecurity Project - Machine Learning based Intrusion Detection System for IEC61850-MMS Profile

Contents:
- Server_APP - High-interaction Honeypot 
- Inline Machine Learning based detection tool 
- AttackClient 

Tested on Debian Linux - Ubuntu and Rpi3  

Prerequisities:
- apt-get update
- apt-get install libsqlite3-dev build-essential iptables-persistent git tcpdump python3-pip doxygen cmake swig
- apt-get install autoconf automake libtool pkg-config libpcap-dev python3-pip
- pip3 install pandas sklearn numpy sklearn-deltatfidf cffi pickle argparse

Setup:
- cd opt;
- git clone https://github.com/ArtWachowski/iec_61850.git;
- cd iec_61850;
- cmake -DBUILD_PYTHON_BINDINGS=ON -DBUILD_EXAMPLES=ON -DDEBUG=OFF -DCONFIG_ACTIVATE_TCP_KEEPALIVE=OFF . 
- make 
- make install
- cp examples/server_app/server_app /usr/bin/;
- server_app &
- LD_LIBRARY_PATH=/usr/local/lib
- export LD_LIBRARY_PATH
- cd CC_SNIFFER
- gcc cc_sniffer.c -o cc_sniffer -I/usr/include/python3.6m -lpython3.6m
- cp iec61850/cc_sniffer /usr/bin/;
- echo 'Finished';

Run IEC61850_pot:
server_app

Run cc_snifer tool:
cc_sniffer

Run attackclient.py from iec61850 directory. 

libiec61850 is an open-source (GPLv3) implementation of an IEC 61850 client and server library implementing the protocols MMS, GOOSE and SV, credit goes to MZ_Autoimation.
