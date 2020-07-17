## Overview

Road Map - IEC61850_CS Project
- IEC61850 Server_APP + Test Client 'Server app is a frankenstein of multiple server_examples so as client, programming pathology' 
- IDS _ Snort Dynamic Pre_Processor IEC61850 extension - basic detection 
- Machine Learing - Inteligent IDS

Dependencies:
- apt-get update
- apt-get install libsqlite3-dev build-essential iptables-persistent git tcpdump python3-pip doxygen cmake swig 

Setup:
- cd opt;
- git clone https://github.com/ArtWachowski/iec_61850.git;
- cd iec_61850;
- make clean* - to remove old stuff
- make examples; 
- cp /opt/iec_61850/examples/server_app/server_app /usr/bin/;

Install python links  
- cmake -DBUILD_PYTHON_BINDINGS=ON -DBUILD_EXAMPLES=OFF (TODO Building Client app is failing) 
- make 
- make install

Problemes with LD's ?? 
- LD_LIBRARY_PATH=/usr/local/lib
- export LD_LIBRARY_PATH
Ref: https://stackoverflow.com/questions/480764/linux-error-while-loading-shared-libraries-cannot-open-shared-object-file-no-s

Run:
server_app


libiec61850 is an open-source (GPLv3) implementation of an IEC 61850 client and server library implementing the protocols MMS, GOOSE and SV. 
It is implemented in C (according to the C99 standard) to provide maximum portability. 
It can be used to implement IEC 61850 compliant client and server applications on embedded systems and PCs running Linux, Windows, and MacOS. 
Included is a set of simple example applications that can be used as a starting point to implement own IEC 61850 compliant devices or to communicate with IEC 61850 devices. 
The library has been successfully used in many commercial software products and devices. Credit goes to MZ_Autoimation
