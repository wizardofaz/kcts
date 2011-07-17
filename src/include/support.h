#ifndef SUPPORT_H
#define SUPPORT_H

#include <fstream>

#include <math.h>
#ifndef WIN32
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#endif

#include "kcts.h"
#include "IOspec.h"

#include "cstack.h"
#include "kcts_io.h"
#include "status.h"

#include "serialcomm.h"
#include <FL/fl_show_colormap.H>
#include <FL/fl_ask.H>

extern void cbExit();

extern void readKachinaVersion();
extern void sendSerialNumber();

extern void cbOkCommsDialog();

extern void about();

extern void updateRcvSignal( int data);
extern void updateTempDisplay(int data);

extern void startProcessing(void *d);

extern void readKachinaVersion();
extern void sendSerialNumber();
extern void readSMeterData();
extern void sendSMeterData();
extern void performSMeterCalibration();
extern void readFreqRefData();
extern void sendFreqRefData();
extern void performFreqRefCalibration();
extern void readPhaseData();
extern void sendPhaseData();
extern void performPhaseCalibration();
extern void readCarrierData();
extern void sendCarrierData();
extern void performCarrierBalance();
extern void readOnTimeData();
extern void clearOnTimeData();
extern char * readDVM( char *cmd );
extern void readAGC();
extern void readLockDet1();
extern void readLockDet2();
extern void readFwdPwr();
extern void readRefPwr();
extern void readPhaseDet();
extern void readTxAudio();
extern void readTempA();
extern void readTempB();
extern void readTCXO();
extern void readAll();
extern void openFile();
extern void saveFile();
extern void saveFileAs();

#endif
