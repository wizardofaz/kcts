#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>

#include "support.h"
#include "kcts.h"
#include "kcts_io.h"
#include "util.h"
#include "config.h"
#include "status.h"
#include "debug.h"

using namespace std;

//=====================================================================
// Moving average filter
//
// Simple in concept, sublime in implementation ... the fastest filter
// in the west.  Also optimal for the processing of time domain signals
// characterized by a transition edge.  The is the perfect signal filter
// for CW, RTTY and other signals of that type.  For a given filter size
// it provides the greatest s/n improvement while retaining the sharpest
// leading edge on the filtered signal.
//=====================================================================

class Cmovavg {
#define MAXMOVAVG 2048
private:
	double	*in;
	double	out;
	int		len, pint;
	bool	empty;
public:
	Cmovavg(int filtlen);
	~Cmovavg();
	double run(double a);
	void setLength(int filtlen);
	void reset();
};

Cmovavg::Cmovavg (int filtlen)
{
	len = filtlen;
	in = new double[len];
	empty = true;
}

Cmovavg::~Cmovavg()
{
	if (in) delete [] in;
}

double Cmovavg::run(double a)
{
	if (empty) {
		empty = false;
		for (int i = 0; i < len; i++) {
			in[i] = a;
		}
		out = a * len;
		pint = 0;
		return a;
	}
	out = out - in[pint] + a;
	in[pint++] = a;
	pint %= len;
	return out / len;
}

void Cmovavg::setLength(int filtlen)
{
	if (filtlen > len) {
		delete [] in;
		in = new double[filtlen];
	}
	len = filtlen;
	empty = true;
}

void Cmovavg::reset()
{
	empty = true;
}

//======================================================================
// Comm port dialog
//======================================================================
Fl_Double_Window *dlgCommsConfig = NULL;

#define COMMPORT_TABLESIZE 20

string ttyport;
string commPortTable[COMMPORT_TABLESIZE];
string sCommPorts;

int  commportnbr = 0;
int  iNbrCommPorts  = 0;
char szttyport[20] = "";
bool commport_table_empty = true;

bool waitfordialog = false;

void clear_combos()
{
	for (int i = 0; i < COMMPORT_TABLESIZE; i++)
		commPortTable[i].clear();
	commPortTable[0] = "TEST";
	sCommPorts = "TEST";
	iNbrCommPorts = 0;
}

void add_combos(char *port)
{
	iNbrCommPorts++;
	if (iNbrCommPorts >= COMMPORT_TABLESIZE) return;

	commPortTable[iNbrCommPorts] = port;
	sCommPorts.append("|").append(port);
}

//======================================================================
// WIN32 init_port_combos
//======================================================================

#ifdef __WIN32__
static bool open_serial(const char* dev)
{
	bool ret = false;
	HANDLE fd = CreateFile(dev, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if (fd != INVALID_HANDLE_VALUE) {
		CloseHandle(fd);
		ret = true;
	}
	return ret;
}

#  define TTY_MAX 255
void initCommPortTable()
{
	clear_combos();

	char ttyname[21];
	const char tty_fmt[] = "//./COM%u";

	for (unsigned j = 0; j < TTY_MAX; j++) {
		snprintf(ttyname, sizeof(ttyname), tty_fmt, j);
		if (!open_serial(ttyname))
			continue;
		snprintf(ttyname, sizeof(ttyname), "COM%u", j);
		LOG_WARN("Found serial port %s", ttyname);
		add_combos(ttyname);
	}
}
#endif //__WIN32__

//======================================================================
// Linux init_port_combos
//======================================================================

#ifdef __linux__
#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif
#  define TTY_MAX 8

void initCommPortTable()
{
	struct stat st;
	char ttyname[PATH_MAX + 1];

	clear_combos();

	const char* tty_fmt[] = {
		"/dev/ttyS%u",
		"/dev/ttyUSB%u",
		"/dev/usb/ttyUSB%u"
	};
	LOG_WARN("%s", "Serial port discovery via 'stat'");
	for (size_t i = 0; i < sizeof(tty_fmt)/sizeof(*tty_fmt); i++) {
		for (unsigned j = 0; j < TTY_MAX; j++) {
			snprintf(ttyname, sizeof(ttyname), tty_fmt[i], j);
			if ( !(stat(ttyname, &st) == 0 && S_ISCHR(st.st_mode)) )
				continue;

			LOG_WARN("Found serial port %s", ttyname);
			add_combos(ttyname);
		}
	}

}
#endif // __linux__

//======================================================================
// APPLE init_port_combos
//======================================================================

#ifdef __APPLE__
#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif

void initCommPortTable()
{
	clear_combos();

	struct stat st;

	const char* tty_fmt[] = {
		"/dev/cu.*",
		"/dev/tty.*"
	};

	glob_t gbuf;

	for (size_t i = 0; i < sizeof(tty_fmt)/sizeof(*tty_fmt); i++) {
		glob(tty_fmt[i], 0, NULL, &gbuf);
		for (size_t j = 0; j < gbuf.gl_pathc; j++) {
			if ( !(stat(gbuf.gl_pathv[j], &st) == 0 && S_ISCHR(st.st_mode)) ||
			     strstr(gbuf.gl_pathv[j], "modem") )
				continue;
			LOG_WARN("Found serial port %s", gbuf.gl_pathv[j]);
			add_combos(gbuf.gl_pathv[j]);
		}
		globfree(&gbuf);
	}
}
#endif //__APPLE__

//======================================================================
// FreeBSD init_port_combos
//======================================================================

#ifdef __FreeBSD__
#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif
#  define TTY_MAX 8

void initCommPortTable()
{
	int retval;
	struct stat st;
	char ttyname[PATH_MAX + 1];
	const char* tty_fmt[] = {
		"/dev/ttyd%u"
	};

	clear_combos();

	for (size_t i = 0; i < sizeof(tty_fmt)/sizeof(*tty_fmt); i++) {
		for (unsigned j = 0; j < TTY_MAX; j++) {
			snprintf(ttyname, sizeof(ttyname), tty_fmt[i], j);
			if ( !(stat(ttyname, &st) == 0 && S_ISCHR(st.st_mode)) )
				continue;
			LOG_WARN("Found serial port %s", ttyname);
			add_combos(ttyname);
		}
	}
}
#endif //__FreeBSD__

void cbOkCommsDialog()
{
	dlgCommsConfig->hide();
	waitfordialog = false;
	commportnbr = selectCommPort->value();
	ttyport =  commPortTable[commportnbr];
	if (ttyport == "TEST")
		kcts_serial.ClosePort();
	else if (startComms(ttyport.c_str(), 9600) == 0) {
		fl_message("%s not available", ttyport.c_str());
		exit(1);
	}


}

void setCommsPort()
{
	if (dlgCommsConfig == NULL)
		dlgCommsConfig = CommsDialog();
	if (commport_table_empty) {
		initCommPortTable();
		selectCommPort->add(sCommPorts.c_str());
		commport_table_empty = false;
	}
	commportnbr = 0;
	selectCommPort->value(commportnbr);
	waitfordialog = true;
	dlgCommsConfig->show();
	while (waitfordialog) Fl::wait();
}

void about()
{
	fl_message("\
Test Suite for Kachina 505\n\
Copyright W1HKJ, 2011\n\
Version %s\n\
w1hkj@@w1hkj.com", PACKAGE_VERSION);
}

//----------------------------------------------------------------------
// Radio sends telemetry data at a 50 msec rate
//----------------------------------------------------------------------
Cmovavg RxSigFilter(20);
double avgSignal = 0.0;

void updateRcvSignal( int data)
{
	char szDisp[10];
	avgSignal = RxSigFilter.run(data);
	snprintf(szDisp, sizeof(szDisp), "%4.0f dB", -avgSignal);
	txtRxSigLevel->value(szDisp);
	txtRxSigLevel->redraw();
}

bool dispCent = true;
char Centigrade[] = "C";
char Fahrenheit[] = "F";

Cmovavg TempFilter(20);
double avgTemp = 0.0;

void updateTempDisplay(int data)
{
	double temp = (data - 220)*2.5 + 17.5;
	avgTemp = TempFilter.run(temp);
	char buff[5];
	if (!dispCent) {
		avgTemp = avgTemp * 9 / 5 + 32.0;
		snprintf(buff, sizeof(buff), "%3.0f F", avgTemp);
	} else
		snprintf(buff, sizeof(buff), "%3.0f C", avgTemp);
	txtTemperature->value(buff);
}

bool exit_telemetry = false;

void parseTelemetry(void *)
{
	unsigned char data;
	while(commstack.pop(data)) {
		LOG_DEBUG("telemetry %d", data);
		if (data < 128)
			updateRcvSignal(data);
		else if (data > 219 && data < 250)
			updateTempDisplay(data);
	}
}


void * telemetry_thread_loop(void *d)
{
	char buff[20];
	int num = 0;
	for (;;) {
		MilliSleep(10);
		if (exit_telemetry) break;
		pthread_mutex_lock(&mutex_serial);
			num = kcts_serial.ReadBuffer (buff, 1);
		pthread_mutex_unlock(&mutex_serial);
		if (num) commstack.push((unsigned char)buff[0]);
	}
	return NULL;
}

//----------------------------------------------------------------------
// watchdog timer sends a NOOP to xcvr every 15 seconds
//----------------------------------------------------------------------

bool exit_watchdog = false;

void * watchdog_thread_loop(void *d)
{
	static int count = 1500;
	for (;;) {
		if (exit_watchdog) break;
		MilliSleep(10);
		if (--count == 0) {
			pthread_mutex_lock(&mutex_watchdog);
			setXcvrNOOP();
			pthread_mutex_unlock(&mutex_watchdog);
			count = 1500;
		}
	}
	return NULL;
}

//----------------------------------------------------------------------

void startProcessing(void *d)
{
	setCommsPort();

	watchdog_thread = new pthread_t;
	if (pthread_create(watchdog_thread, NULL, watchdog_thread_loop, NULL)) {
		perror("pthread_create watchdog");
		exit(EXIT_FAILURE);
	}

	telemetry_thread = new pthread_t;
	if (pthread_create(telemetry_thread, NULL, telemetry_thread_loop, NULL)) {
		perror("pthread_create telemetry");
		exit(EXIT_FAILURE);
	}
	Fl::add_idle(parseTelemetry);

	readKachinaVersion();
	debug::show();
}

void cbExit()
{
// close watchdog
	pthread_mutex_lock(&mutex_watchdog);
	exit_watchdog = true;
	pthread_mutex_unlock(&mutex_watchdog);
	pthread_join(*watchdog_thread, NULL);

// close telemetry
	pthread_mutex_lock(&mutex_telemetry);
	exit_telemetry = true;
	pthread_mutex_unlock(&mutex_telemetry);
	pthread_join(*telemetry_thread, NULL);

	exit(0);
}

//----------------------------------------------------------------------

long RigSerNbr = 0;
char szRigSerNbr[20] = "";
int  RigFirm[2] = {0,0};
char szRigFirm[5] = "";
int  RigHard[2] = {0,0};
char szRigHard[5] = "";

void readKachinaVersion()
{
	unsigned char buffer[10];
	RequestData (cmdK_RSER, buffer, 10);
	LOG_WARN("\n%s", retval.c_str());

	RigSerNbr = (((buffer[0]*256 + buffer[1])*256) + buffer[2])*256 + buffer[3];
	RigFirm[0] = buffer[6]; RigFirm[1] = buffer[7];
	RigHard[0] = buffer[4]; RigHard[1] = buffer[5];
	snprintf(szRigSerNbr, sizeof(szRigSerNbr), "%ld", RigSerNbr);
	txtSerialNumber->value(szRigSerNbr);
	snprintf(szRigFirm, sizeof(szRigFirm), "%d.%d", RigFirm[0], RigFirm[1]);
	txtFirmware->value(szRigFirm);
	snprintf(szRigHard, sizeof(szRigHard), "%c.%d", RigHard[0], RigHard[1]);
	txtHardware->value(szRigHard);
}

void sendSerialNumber()
{
	unsigned long int sernbr;
	unsigned char serbuff[4];
	sernbr = atol(txtSerialNumber->value());
	serbuff[3] = sernbr & 0xFF;
	sernbr /= 256;
	serbuff[2] = sernbr & 0xFF;
	sernbr /= 256;
	serbuff[1] = sernbr & 0xFF;
	serbuff[0] = (sernbr / 256) & 0xFF;
	sendData (cmdK_cmdJ, serbuff, 4);
}

void readSMeterData()
{
	unsigned char data[18];
	char line[256];
	int chksum;
	double xvals[16], yvals[16];
	int sgndata;

	RequestData (cmdK_RSMTR, data, 18);
	LOG_WARN("\n%s", retval.c_str());

	chksum = 0;
	for (int i = 0; i < 16; i++)
			chksum += data[i];
	if (chksum == 256*data[16]+data[17]) {
		for (int j = 0; j < 16; j++) {
			sgndata = data[15-j] > 128 ? data[15-j] - 256 : data[15-j];
			snprintf(line, sizeof(line), "%4d", sgndata);
			sMeter[j]->value(line);
			xvals[j] = j;
			yvals[j] = sgndata;
		}
		pltSMeter->xMin(0.0); pltSMeter->xMax(15.0);
		pltSMeter->yMin(-128.0); pltSMeter->yMax(128.0);
		pltSMeter->plotXY(xvals, yvals, 16);
	}
	pltSMeter->redraw();
}

void sendSMeterData()
{
	unsigned char sLine[18];
	int data;
	int chksum = 0;

	for (int i = 0; i < 16; i++) {
		data = atoi(sMeter[15-i]->value());
		if (data < 0) data += 256;
		chksum += data;
		sLine[i] = data & 0xFF;
	}
	sLine[16] = (chksum / 256) & 0xFF;
	sLine[17] = chksum & 0xFF;

	sendData (cmdK_SSMTR, sLine, 18);

}

void performSMeterCalibration()
{
	sendCommand(cmdK_PSMTR);
}

void readFreqRefData()
{
	unsigned char data[34];
	char line[256];
	int chksum;
	double xvals[32], yvals[32];

	RequestData (cmdK_RFCAL, data, 34);
	LOG_WARN("\n%s", retval.c_str());

	chksum = 0;
	for (int i = 0; i < 32; i++)
			chksum += data[i];
	if (chksum == 256*data[32]+data[33]) {
		for (int j = 0; j < 32; j++) {
			snprintf(line, sizeof(line), "%d", data[j]);
			freqRef[j]->value(line);
			xvals[j] = j;
			yvals[j] = data[j];
		}
		pltFreqRef->xMin(0.0); pltFreqRef->xMax(31.0);
		pltFreqRef->yMin(0.0); pltFreqRef->yMax(256.0);
		pltFreqRef->plotXY ( xvals, yvals, 32 );
	}
	pltFreqRef->redraw();
}

void sendFreqRefData()
{
	unsigned int val;
	unsigned char frefbuff[34];
	unsigned int chksum = 0;
	for (int i = 0; i < 32; i++) {
		frefbuff[i] = val = atol(freqRef[i]->value()) & 0xFF;
		chksum += val;
	}
	frefbuff[33] = chksum & 0xFF;
	frefbuff[32] = (chksum / 256) & 0xFF;
	sendData (cmdK_SFCAL, frefbuff, 34);
}

void performFreqRefCalibration()
{
	sendCommand(cmdK_PFCAL);
}

void readPhaseData()
{
	unsigned char data[18];
	char line[256];
	int chksum;
	double xvals[16], yvals[16];

	RequestData (cmdK_RPCAL, data, 18);
	LOG_WARN("\n%s", retval.c_str());

	chksum = 0;
	for (int i = 0; i < 16; i++)
			chksum += data[i];
	if (chksum == 256*data[16]+data[17]) {
		for (int j = 0; j < 16; j++) {
			snprintf(line, sizeof(line), "%d", data[j]);
			phase[j]->value(line);
			xvals[j] = j;
			yvals[j] = data[j] > 127 ? data[j] - 256 : data[j];
		}
		pltPhase->xMin(0.0); pltPhase->xMax(15.0);
		pltPhase->yMin(-128); pltPhase->yMax(128);
		pltPhase->plotXY( xvals, yvals, 16 );
	}
	pltPhase->redraw();
}

void sendPhaseData()
{
	unsigned int val;
	unsigned char phasebuff[18];
	unsigned int chksum = 0;
	for (int i = 0; i < 16; i++) {
		phasebuff[i] = val = atol(phase[i]->value()) & 0xFF;
		chksum += val;
	}
	phasebuff[17] = chksum & 0xFF;
	phasebuff[16] = (chksum / 256) & 0xFF;
	sendData (cmdK_SPCAL, phasebuff, 18);
}

void performPhaseCalibration()
{
	sendCommand(cmdK_PPCAL);
}


void readCarrierData()
{
	unsigned char data[2];
	char szVal[10];

	RequestData (cmdK_RCBAL, data, 2);
	LOG_WARN("%s", retval.c_str());
	int val = data[1] + 256 * data[0];
	if (val > 32768) val -= 65536;
	snprintf(szVal, sizeof(szVal), "%6d", val);
	valCarrierBalance->value(szVal);
}

void sendCarrierData()
{
	unsigned char cbdata[2];
	unsigned int val = atol(valCarrierBalance->value());
	cbdata[1] = val & 0xFF;
	cbdata[0] = (val / 256) & 0xFF;
	sendData (cmdK_SCBAL, cbdata, 2);
}

void performCarrierBalance()
{
	sendCommand(cmdK_PCBAL);
}

void readOnTimeData()
{
	unsigned char data[6];
	long int hours;
	int chksum = 0;
	char szHrs[11];

	for (int i = 0; i < 6; data[i++] = 0);

	string cmd = cmdK_cmdF;
	RequestData ((char *)cmd.c_str(), data, 6);
	LOG_WARN("%s", retval.c_str());

	for (int i = 0; i < 4; i++)
			chksum += data[i];
	if (chksum == 256*data[4]+data[5]) {
		hours = (((data[0]*256 + data[1])*256) + data[2])*256 + data[3];
		snprintf(szHrs, sizeof(szHrs), "%10ld", hours);
		txtOpHours->value(szHrs);
	}

}

void clearOnTimeData()
{
}

char * readDVM( char *cmd )
{
	unsigned char cDVM[2];
	float valDVM;
	static char szDVM[7];

	strcpy(szDVM, "");

	RequestData (cmd, cDVM, 1);
	if (cDVM[0] > 0x7f)
		valDVM = -(cDVM[0] - 128)/16.0;
	else
		valDVM = cDVM[0]/16.0;
	snprintf(szDVM, sizeof(szDVM), "%6.1f", valDVM);
	return szDVM;
}

void readAGC()
{
//	txtAGC->value( readDVM(cmdK_SAGC) );
}

void readLockDet1()
{
//	txtLockDet1->value( readDVM(cmdK_cmd2) );
}

void readLockDet2()
{
//	txtLockDet2->value( readDVM(cmdK_cmd3) );
}

void readFwdPwr()
{
//	txtFwdPwr->value( readDVM(cmdK_Qfwd) );
}

void readRefPwr()
{
//	txtRefPwr->value( readDVM(cmdK_Qref) );
}

void readPhaseDet()
{
//	txtPhaseDet->value( readDVM(cmdK_Qphs) );
}

void readTxAudio()
{
//	txtTxAudio->value( readDVM(cmdK_cmd7) );
}

void readTempA()
{
//	txtTempA->value( readDVM(cmdK_Qt_A) );
}

void readTempB()
{
//	txtTempB->value( readDVM(cmdK_Qt_B) );
}

void readTCXO()
{
	txtTCXO->value( readDVM(cmdK_cmdM) );
	LOG_WARN("%s", retval.c_str());
}

void readAll()
{
	readKachinaVersion();
	MilliSleep(50);
	readSMeterData();
	MilliSleep(50);
	readFreqRefData();
	MilliSleep(50);
	readPhaseData();
	MilliSleep(50);
	readCarrierData();
//	MilliSleep(50);
//	readOnTimeData();
	MilliSleep(50);
	readTCXO();
}


char filename[200] = "";

void openFile()
{
	string defaultname = homedir;
	defaultname.append("SRAM.txt");
	char *fname = fl_file_chooser( "Select input file", "*.txt", defaultname.c_str());
	char text[80];
	double xvals[32], yvals[32];


	if (fname != NULL)
		strcpy(filename, fname);
	ifstream input(filename);
	if (!input) {
		fl_message ("Could not open %s", filename);
		return;
	}

	input >> text;
	input >> text;
	txtSerialNumber->value(text);

	input >> text;
	for (int i = 0; i < 16; i++) {
		input >> text;
		sMeter[i]->value(text);
		xvals[i] = i;
		yvals[i] = atol(text);
	}
	pltSMeter->xMin(0.0); pltSMeter->xMax(15.0);
	pltSMeter->yMin(-128.0); pltSMeter->yMax(128.0);
	pltSMeter->plotXY(xvals, yvals, 16);
	pltSMeter->redraw();

	input >> text;
	for (int i = 0; i < 32; i++) {
		input >> text;
		freqRef[i]->value(text);
		xvals[i] = i;
		yvals[i] = atol(text);
	}
	pltFreqRef->xMin(0.0); pltFreqRef->xMax(31.0);
	pltFreqRef->yMin(0.0); pltFreqRef->yMax(256.0);
	pltFreqRef->plotXY ( xvals, yvals, 32 );
	pltFreqRef->redraw();

	input >> text;
	for (int i = 0; i < 16; i++) {
		int val;
		input >> text;
		phase[i]->value(text);
		xvals[i] = i;
		val = atol(text);
		val = val > 128 ? val - 256 : val;
		yvals[i] = val;
	}
	pltPhase->xMin(0.0); pltPhase->xMax(15.0);
	pltPhase->yMin(-128); pltPhase->yMax(128);
	pltPhase->plotXY( xvals, yvals, 16 );
	pltPhase->redraw();

	input >> text;
	input >> text;
	valCarrierBalance->value(text);

	input.close();
}

void saveFile()
{
	string defaultname = homedir;
	defaultname.append("SRAM.txt");
	char *fname;
	if (filename[0] == 0) {
		fname = fl_file_chooser( "Select save file", "*.txt", defaultname.c_str());
		if (fname != NULL)
		strcpy(filename, fname);
	}
	ofstream output(filename);
	if (!output) {
		fl_message ("Could not open %s", filename);
		return;
	}
	output << "Serial_number:" << endl;
	output << txtSerialNumber->value() << endl;
	output << "Smeter_data:" << endl;
	for (int i = 0; i < 16; i++)
		output << sMeter[i]->value() << " ";
	output << endl;
	output << "Freq_ref_data:" << endl;
	for (int i = 0; i < 32; i++)
		output << freqRef[i]->value() << " ";
	output << endl;
	output << "Phase_data:" << endl;
	for (int i = 0; i < 16; i++)
		output << phase[i]->value() << " ";
	output << endl;
	output << "Carrier_data:" << endl;
	output << valCarrierBalance->value() << endl;
	output.close();
}

void saveFileAs()
{
	char *fname;
	fname = fl_file_chooser( "Select save as file", "*.txt", homedir.c_str());
	if (fname != NULL)
		strcpy(filename, fname);
	ofstream output(filename);
	if (!output) {
		fl_message ("Could not open %s", filename);
		return;
	}
	output << "Serial_number:" << endl;
	output << txtSerialNumber->value() << endl;
	output << "Smeter_data:" << endl;
	for (int i = 0; i < 16; i++)
		output << sMeter[i]->value() << " ";
	output << endl;
	output << "Freq_ref_data:" << endl;
	for (int i = 0; i < 32; i++)
		output << freqRef[i]->value() << " ";
	output << endl;
	output << "Phase_data:" << endl;
	for (int i = 0; i < 16; i++)
		output << phase[i]->value() << " ";
	output << endl;
	output << "Carrier_data:" << endl;
	output << valCarrierBalance->value() << endl;
	output.close();
}
//----------------------------------------------------------------------
