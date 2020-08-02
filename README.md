## Overview
IEC61850_CyberSecurity Project

- IEC61850 Server_APP 
- IDS _  IEC61850 CS threat detection model

Dependencies:
- apt-get update
- apt-get install libsqlite3-dev build-essential iptables-persistent git tcpdump python3-pip doxygen cmake swig
- apt-get install autoconf automake libtool pkg-config libpcap-dev python3-pip
- pip3 install pandas sklearn numpy sklearn-deltatfidf

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


libiec61850 is an open-source (GPLv3) implementation of an IEC 61850 client and server library implementing the protocols MMS, GOOSE and SV. 
It is implemented in C (according to the C99 standard) to provide maximum portability. 
It can be used to implement IEC 61850 compliant client and server applications on embedded systems and PCs running Linux, Windows, and MacOS. 
Included is a set of simple example applications that can be used as a starting point to implement own IEC 61850 compliant devices or to communicate with IEC 61850 devices. 
The library has been successfully used in many commercial software products and devices. 
- IEC61850 credit goes to MZ_Autoimation
