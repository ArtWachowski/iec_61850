/*
 * client.c
 * _logging
 * _control a device  
 */

#include "iec61850_client.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "hal_thread.h"

static void
printJournalEntries(LinkedList journalEntries)
{
    char buf[1024];

    LinkedList journalEntriesElem = LinkedList_getNext(journalEntries);

    while (journalEntriesElem != NULL) {

        MmsJournalEntry journalEntry = (MmsJournalEntry) LinkedList_getData(journalEntriesElem);

        MmsValue_printToBuffer(MmsJournalEntry_getEntryID(journalEntry), buf, 1024);
        printf("EntryID: %s\n", buf);
        MmsValue_printToBuffer(MmsJournalEntry_getOccurenceTime(journalEntry), buf, 1024);
        printf("  occurence time: %s\n", buf);

        LinkedList journalVariableElem = LinkedList_getNext(journalEntry->journalVariables);

        while (journalVariableElem != NULL) {

            MmsJournalVariable journalVariable = (MmsJournalVariable) LinkedList_getData(journalVariableElem);

            printf("   variable-tag: %s\n", MmsJournalVariable_getTag(journalVariable));
            MmsValue_printToBuffer(MmsJournalVariable_getValue(journalVariable), buf, 1024);
            printf("   variable-value: %s\n", buf);

            journalVariableElem = LinkedList_getNext(journalVariableElem);
        }

        journalEntriesElem = LinkedList_getNext(journalEntriesElem);
    }
}

static void commandTerminationHandler(void *parameter, ControlObjectClient connection)
{


    LastApplError lastApplError = ControlObjectClient_getLastApplError(connection);

    /* if lastApplError.error != 0 this indicates a CommandTermination- */
    if (lastApplError.error != 0) {
        printf("Received CommandTermination-.\n");
        printf(" LastApplError: %i\n", lastApplError.error);
        printf("      addCause: %i\n", lastApplError.addCause);
    }
    else
        printf("Received CommandTermination+.\n");
}


int main(int argc, char** argv) {

    char* hostname;
    int tcpPort = 102;

    if (argc > 1)
        hostname = argv[1];
    else
        hostname = "localhost";

    if (argc > 2)
        tcpPort = atoi(argv[2]);

    char* logRef = "simpleIOGenericIO/LLN0$EventLog";

    IedClientError error;

    IedConnection con = IedConnection_create();

    IedConnection_connect(con, &error, hostname, tcpPort);

    printf("PERFORMING LOG POLLING");

    if (error == IED_ERROR_OK) {

        /* read list of logs in LN (optional - if you don't know the existing logs) */
        LinkedList logs = IedConnection_getLogicalNodeDirectory(con, &error, "simpleIOGenericIO/LLN0", ACSI_CLASS_LOG);

        if (error == IED_ERROR_OK) {

            if (LinkedList_size(logs) > 0) {
                printf("Found logs in LN simpleIOGenericIO/LLN0:\n");

                LinkedList log = LinkedList_getNext(logs);

                while (log != NULL) {
                    char* logName = (char*) LinkedList_getData(log);

                    printf("  %s\n", logName);

                    log = LinkedList_getNext(log);
                }
            }
            else {
                printf("No logs found\n");
            }

            LinkedList_destroy(logs);
        }

        /* read log control block (using the generic read function) */
        MmsValue* lcbValue = IedConnection_readObject(con, &error, "simpleIOGenericIO/LLN0.EventLog", IEC61850_FC_LG);

        if ((error == IED_ERROR_OK) && (MmsValue_getType(lcbValue) != MMS_DATA_ACCESS_ERROR)) {
	    
	    char printBuf[1024];

            printf("LCB values: %s\n", printBuf);
            
            MmsValue_printToBuffer(lcbValue, printBuf, 1024);

            printf("LCB values: %s\n", printBuf);

            MmsValue* oldEntryTm = MmsValue_getElement(lcbValue, 3);
            MmsValue* oldEntry = MmsValue_getElement(lcbValue, 5);

            uint64_t timestamp = MmsValue_getUtcTimeInMs(oldEntryTm);

            bool moreFollows;

            /*
             * read the log contents. Be aware that the logRef uses the '$' sign as separator between the LN and
             * the log name! This is in contrast to the LCB object reference above.
             */
            LinkedList logEntries = IedConnection_queryLogAfter(con, &error, "simpleIOGenericIO/LLN0$EventLog", oldEntry, timestamp, &moreFollows);

            if (error == IED_ERROR_OK) {
                printJournalEntries(logEntries);

                LinkedList_destroyDeep(logEntries, (LinkedListValueDeleteFunction) MmsJournalEntry_destroy);
            }
            else
                printf("QueryLog failed (error code: %i)!\n", error);

            //TODO handle moreFollows

            MmsValue_delete(lcbValue);
        }
        else
            printf("Read LCB failed!\n");

        }

        if (error == IED_ERROR_OK) {

        /************************
         * Direct control
         ***********************/
        
        ControlObjectClient control
            = ControlObjectClient_create("simpleIOGenericIO/GGIO1.SPCSO6", con);

        MmsValue* ctlVal = MmsValue_newBoolean(true);

        ControlObjectClient_setOrigin(control, NULL, 3);

        if (ControlObjectClient_operate(control, ctlVal, 1 /* operate now */)) {
            printf("simpleIOGenericIO/GGIO1.SPCSO6 operated successfully\n");
        }
        else {
            printf("failed to operate simpleIOGenericIO/GGIO1.SPCSO6\n");
        }

        MmsValue_delete(ctlVal);

        ControlObjectClient_destroy(control);

        /* Check if status value has changed */

        MmsValue* stVal = IedConnection_readObject(con, &error, "simpleIOGenericIO/GGIO1.SPCSO6.stVal", IEC61850_FC_ST);

        if (error == IED_ERROR_OK) {
            bool state = MmsValue_getBoolean(stVal);
            MmsValue_delete(stVal);

            printf("New status of simpleIOGenericIO/GGIO1.SPCSO6.stVal: %i\n", state);
        }
        else {
            printf("Reading status for simpleIOGenericIO/GGIO1.SPCSO6 failed!\n");
        }


        /************************
         * Select before operate
         ***********************/

        control = ControlObjectClient_create("simpleIOGenericIO/GGIO1.SPCSO2", con);

        if (ControlObjectClient_select(control)) {

            ctlVal = MmsValue_newBoolean(true);

            if (ControlObjectClient_operate(control, ctlVal, 0 /* operate now */)) {
                printf("simpleIOGenericIO/GGIO1.SPCSO2 operated successfully\n");
            }
            else {
                printf("failed to operate simpleIOGenericIO/GGIO1.SPCSO2!\n");
            }

            MmsValue_delete(ctlVal);
        }
        else {
            printf("failed to select simpleIOGenericIO/GGIO1.SPCSO2!\n");
        }

        ControlObjectClient_destroy(control);


        /****************************************
         * Direct control with enhanced security
         ****************************************/

        control = ControlObjectClient_create("simpleIOGenericIO/GGIO1.SPCSO7", con);

        ControlObjectClient_setCommandTerminationHandler(control, commandTerminationHandler, NULL);

        ctlVal = MmsValue_newBoolean(true);

        if (ControlObjectClient_operate(control, ctlVal, 0 /* operate now */)) {
            printf("simpleIOGenericIO/GGIO1.SPCSO7 operated successfully\n");
        }
        else {
            printf("failed to operate simpleIOGenericIO/GGIO1.SPCSO7\n");
        }

        MmsValue_delete(ctlVal);

        /* Wait for command termination message */
        Thread_sleep(1000);

        ControlObjectClient_destroy(control);

        /* Check if status value has changed */

       stVal = IedConnection_readObject(con, &error, "simpleIOGenericIO/GGIO1.SPCSO7.stVal", IEC61850_FC_ST);

        if (error == IED_ERROR_OK) {
            bool state = MmsValue_getBoolean(stVal);

            printf("New status of simpleIOGenericIO/GGIO1.SPCSO7.stVal: %i\n", state);

            MmsValue_delete(stVal);
        }
        else {
            printf("Reading status for simpleIOGenericIO/GGIO1.SPCSO7 failed!\n");
        }

        /***********************************************
         * Select before operate with enhanced security
         ***********************************************/

        control = ControlObjectClient_create("simpleIOGenericIO/GGIO1.SPCSO8", con);

        ControlObjectClient_setCommandTerminationHandler(control, commandTerminationHandler, NULL);

        ctlVal = MmsValue_newBoolean(true);

        if (ControlObjectClient_selectWithValue(control, ctlVal)) {

            if (ControlObjectClient_operate(control, ctlVal, 0 /* operate now */)) {
                printf("simpleIOGenericIO/GGIO1.SPCSO8 operated successfully\n");
            }
            else {
                printf("failed to operate simpleIOGenericIO/GGIO1.SPCSO8!\n");
            }

        }
        else {
            printf("failed to select simpleIOGenericIO/GGIO1.SPCSO8!\n");
        }

        MmsValue_delete(ctlVal);

        /* Wait for command termination message */
        Thread_sleep(1000);

        ControlObjectClient_destroy(control);


        /*********************************************************************
         * Direct control with enhanced security (expect CommandTermination-)
         *********************************************************************/

        control = ControlObjectClient_create("simpleIOGenericIO/GGIO1.SPCSO9", con);

        ControlObjectClient_setCommandTerminationHandler(control, commandTerminationHandler, NULL);

        ctlVal = MmsValue_newBoolean(true);

        if (ControlObjectClient_operate(control, ctlVal, 0 /* operate now */)) {
            printf("simpleIOGenericIO/GGIO1.SPCSO9 operated successfully\n");
        }
        else {
            printf("failed to operate simpleIOGenericIO/GGIO1.SPCSO9\n");
        }

        MmsValue_delete(ctlVal);

        /* Wait for command termination message */
        Thread_sleep(1000);

        ControlObjectClient_destroy(control);


        IedConnection_close(con);

    } else {
    	printf("Connection failed!\n");
    }

    IedConnection_destroy(con);
}


