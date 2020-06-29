/*
 *  server_app.c
 *
 *  - How to use a server with logging service
 *  - How to store arbitrary data in a log
 *
 */

#include "iec61850_server.h"
#include "hal_thread.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "static_model.h"

#include "logging_api.h"

LogStorage
SqliteLogStorage_createInstance(const char* filename);

/* import IEC 61850 device model created from SCL-File */
extern IedModel iedModel;

static int running = 0;
static IedServer iedServer = NULL;

void
sigint_handler(int signalId)
{
    running = 0;
}

static ControlHandlerResult
controlHandlerForBinaryOutput(ControlAction action, void* parameter, MmsValue* value, bool test)
{
    if (test) {
        printf("Received test command\n");
        return CONTROL_RESULT_FAILED;
    }

    if (MmsValue_getType(value) == MMS_BOOLEAN) {
        printf("received binary control command: ");

        if (MmsValue_getBoolean(value))
            printf("on\n");
        else
            printf("off\n");
    }
    else
        return CONTROL_RESULT_FAILED;

    uint64_t timeStamp = Hal_getTimeInMs();

    if (parameter == IEDMODEL_GenericIO_GGIO1_SPCSO1) {
        IedServer_updateUTCTimeAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO1_t, timeStamp);
        IedServer_updateAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO1_stVal, value);
    }

    if (parameter == IEDMODEL_GenericIO_GGIO1_SPCSO5) {
        IedServer_updateUTCTimeAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO5_t, timeStamp);
        IedServer_updateAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO5_stVal, value);
    }

    if (parameter == IEDMODEL_GenericIO_GGIO1_SPCSO3) {
        IedServer_updateUTCTimeAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO3_t, timeStamp);
        IedServer_updateAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO3_stVal, value);
    }

    if (parameter == IEDMODEL_GenericIO_GGIO1_SPCSO4) {
        IedServer_updateUTCTimeAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO4_t, timeStamp);
        IedServer_updateAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO4_stVal, value);
    }

    return CONTROL_RESULT_OK;
}

static void
connectionHandler (IedServer self, ClientConnection connection, bool connected, void* parameter)
{
    if (connected)
        printf("Connection opened\n");
    else
        printf("Connection closed\n");
}

static bool
entryCallback(void* parameter, uint64_t timestamp, uint64_t entryID, bool moreFollow)
{
#if 0
    if (moreFollow)
        printf("Found entry ID:%llu timestamp:%llu\n", entryID, timestamp);
#endif
    return true;
}

static bool
entryDataCallback (void* parameter, const char* dataRef, const uint8_t* data, int dataSize, uint8_t reasonCode, bool moreFollow)
{
#if 0
    if (moreFollow) {
        printf("  EntryData: ref: %s\n", dataRef);

        MmsValue* value = MmsValue_decodeMmsData(data, 0, dataSize);

        char buffer[256];

        MmsValue_printToBuffer(value, buffer, 256);

        printf("  value: %s\n", buffer);

        MmsValue_delete(value);
    }
#endif

    return true;
}

/*
*CONTROL STUFF
*/

static CheckHandlerResult
checkHandler(ControlAction action, void* parameter, MmsValue* ctlVal, bool test, bool interlockCheck)
{
    ClientConnection clientCon = ControlAction_getClientConnection(action);

    if (clientCon) {
        printf("Control from client %s\n", ClientConnection_getPeerAddress(clientCon));
    }
    else {
        printf("clientCon == NULL\n");
    }

    if (ControlAction_isSelect(action))
        printf("check handler called by select command!\n");
    else
        printf("check handler called by operate command!\n");

    if (interlockCheck)
        printf("  with interlock check bit set!\n");

    printf("  ctlNum: %i\n", ControlAction_getCtlNum(action));

    if (parameter == IEDMODEL_GenericIO_GGIO1_SPCSO6)
        return CONTROL_ACCEPTED;

    if (parameter == IEDMODEL_GenericIO_GGIO1_SPCSO2)
        return CONTROL_ACCEPTED;

    if (parameter == IEDMODEL_GenericIO_GGIO1_SPCSO7)
        return CONTROL_ACCEPTED;

    if (parameter == IEDMODEL_GenericIO_GGIO1_SPCSO8)
        return CONTROL_ACCEPTED;

    if (parameter == IEDMODEL_GenericIO_GGIO1_SPCSO9)
        return CONTROL_ACCEPTED;

    return CONTROL_OBJECT_UNDEFINED;
}

static MmsDataAccessError
writeAccessHandler (DataAttribute* dataAttribute, MmsValue* value, ClientConnection connection, void* parameter)
{
    ControlModel ctlModelVal = (ControlModel) MmsValue_toInt32(value);

    /* we only allow status-only and direct-operate */
    if ((ctlModelVal == CONTROL_MODEL_STATUS_ONLY) || (ctlModelVal == CONTROL_MODEL_DIRECT_NORMAL))
    {
        IedServer_updateCtlModel(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO6, ctlModelVal);

        printf("Changed GGIO1.SPCSI.ctlModel to %i\n", ctlModelVal);

        return DATA_ACCESS_ERROR_SUCCESS;
    }
    else {
        return DATA_ACCESS_ERROR_OBJECT_VALUE_INVALID;
    }
}
/*
*end CONTROL STUFF
*/

int
main(int argc, char** argv)
{
    printf("Using libIEC61850 version %s\n", LibIEC61850_getVersionString());

    iedServer = IedServer_create(&iedModel);

    /* Install handler for operate command */
    IedServer_setControlHandler(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO1,
            (ControlHandler) controlHandlerForBinaryOutput,
            IEDMODEL_GenericIO_GGIO1_SPCSO1);

    IedServer_setControlHandler(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO5,
            (ControlHandler) controlHandlerForBinaryOutput,
            IEDMODEL_GenericIO_GGIO1_SPCSO5);

    IedServer_setControlHandler(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO3,
            (ControlHandler) controlHandlerForBinaryOutput,
            IEDMODEL_GenericIO_GGIO1_SPCSO3);

    IedServer_setControlHandler(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO4,
            (ControlHandler) controlHandlerForBinaryOutput,
            IEDMODEL_GenericIO_GGIO1_SPCSO4);

/*
*CONTROL STUFF
*/


    /* Install handler for operate command */
    IedServer_setControlHandler(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO6,
            (ControlHandler) controlHandlerForBinaryOutput,
            IEDMODEL_GenericIO_GGIO1_SPCSO6);

    /*
     * For SPCSO1 we want the user be able to change the control model by online service -
     * so we install a write access handler to change the control model when the client
     * writes to the "ctlModel" attribute.
     */
    IedServer_handleWriteAccess(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO6_ctlModel, writeAccessHandler, NULL);


    IedServer_setControlHandler(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO2,
            (ControlHandler) controlHandlerForBinaryOutput,
            IEDMODEL_GenericIO_GGIO1_SPCSO2);

    /* this is optional - performs operative checks */
    IedServer_setPerformCheckHandler(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO2, checkHandler,
            IEDMODEL_GenericIO_GGIO1_SPCSO2);


    IedServer_setControlHandler(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO6,
            (ControlHandler) controlHandlerForBinaryOutput,
            IEDMODEL_GenericIO_GGIO1_SPCSO6);

    IedServer_setControlHandler(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO7,
            (ControlHandler) controlHandlerForBinaryOutput,
            IEDMODEL_GenericIO_GGIO1_SPCSO7);
 
    /* this is optional - performs operative checks */
    IedServer_setPerformCheckHandler(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO8, checkHandler,
            IEDMODEL_GenericIO_GGIO1_SPCSO8);


    IedServer_setControlHandler(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO9,
            (ControlHandler) controlHandlerForBinaryOutput,
            IEDMODEL_GenericIO_GGIO1_SPCSO9);

    /* this is optional - performs operative checks */
    IedServer_setPerformCheckHandler(iedServer, IEDMODEL_GenericIO_GGIO1_SPCSO9, checkHandler,
            IEDMODEL_GenericIO_GGIO1_SPCSO9);

/*
* FINISH CONTROL STUFF
*/


    IedServer_setConnectionIndicationHandler(iedServer, (IedConnectionIndicationHandler) connectionHandler, NULL);

    LogStorage statusLog = SqliteLogStorage_createInstance("log_status.db");

    LogStorage_setMaxLogEntries(statusLog, 100);

    IedServer_setLogStorage(iedServer, "GenericIO/LLN0$EventLog", statusLog);

#if 1
    uint64_t entryID = LogStorage_addEntry(statusLog, Hal_getTimeInMs());

    MmsValue* value = MmsValue_newIntegerFromInt32(123);

    uint8_t blob[256];

    int blobSize = MmsValue_encodeMmsData(value, blob, 0, true);


    LogStorage_addEntryData(statusLog, entryID, "simpleIOGenerioIO/GPIO1$ST$SPCSO1$stVal", blob, blobSize, 0);

    MmsValue_delete(value);

    value = MmsValue_newUtcTimeByMsTime(Hal_getTimeInMs());

    blobSize = MmsValue_encodeMmsData(value, blob, 0, true);

    MmsValue_delete(value);

    LogStorage_addEntryData(statusLog, entryID, "simpleIOGenerioIO/GPIO1$ST$SPCSO1$t", blob, blobSize, 0);

    LogStorage_getEntries(statusLog, 0, Hal_getTimeInMs(), entryCallback, (LogEntryDataCallback) entryDataCallback, NULL);
#endif


    /* MMS server will be instructed to start listening to client connections. */
    IedServer_start(iedServer, 102);

    if (!IedServer_isRunning(iedServer)) {
        printf("Starting server failed! Exit.\n");
        IedServer_destroy(iedServer);
        exit(-1);
    }

    running = 1;

    signal(SIGINT, sigint_handler);

    float t = 0.f;

    while (running) {
        uint64_t timestamp = Hal_getTimeInMs();

        t += 0.1f;

        float an1 = sinf(t);
        float an2 = sinf(t + 1.f);
        float an3 = sinf(t + 2.f);
        float an4 = sinf(t + 3.f);

        IedServer_lockDataModel(iedServer);

        Timestamp iecTimestamp;

        Timestamp_clearFlags(&iecTimestamp);
        Timestamp_setTimeInMilliseconds(&iecTimestamp, timestamp);
        Timestamp_setLeapSecondKnown(&iecTimestamp, true);

        /* toggle clock-not-synchronized flag in timestamp */
        if (((int) t % 2) == 0)
            Timestamp_setClockNotSynchronized(&iecTimestamp, true);

        IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_AnIn1_t, &iecTimestamp);
        IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_AnIn1_mag_f, an1);

        IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_AnIn2_t, &iecTimestamp);
        IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_AnIn2_mag_f, an2);

        IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_AnIn3_t, &iecTimestamp);
        IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_AnIn3_mag_f, an3);

        IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_AnIn4_t, &iecTimestamp);
        IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_GenericIO_GGIO1_AnIn4_mag_f, an4);

        IedServer_unlockDataModel(iedServer);

        Thread_sleep(100);
    }

    /* stop MMS server - close TCP server socket and all client sockets */
    IedServer_stop(iedServer);

    /* Cleanup - free all resources */
    IedServer_destroy(iedServer);

    /* Release connection to database and free resources */
    LogStorage_destroy(statusLog);

} /* main() */
