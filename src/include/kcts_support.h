#ifndef SUPPORT_H
#define SUPPORT_H

#include <fstream>

#include <math.h>
#ifndef WIN32
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#endif

#include "main.h"
#include "version.h"
#include "IOspec.h"

#include "cstack.h"
#include "k_io.h"

#include "serialcomm.h"
#include <FL/fl_show_colormap.H>
#include <FL/fl_ask.H>

#define LISTSIZE 200

// comment out the following to disable shared memory 
#ifndef WIN32
#define SHARED_MEM 1
#endif

extern bool test;

extern void cbExit();

extern void readKachinaVersion();
extern void sendSerialNumber();

extern void readSMeterData();
extern void sendSMeterData();
extern void performSMeterCalibration();

extern void readFreqRefData();
extern void sendFreqRefData();
extern void performFreqRefCalibration();

extern void readCarrierData();
extern void	sendCarrierData();
extern void performCarrierBalance();

extern void readPhaseData();
extern void sendPhaseData();
extern void performPhaseCalibration();

extern void readOnTimeData();
extern void clearOnTimeData();

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

extern void cbOkCommsDialog();

extern void about();

#endif
