#include <math.h>
#include <iostream>

#include "kcts.h"
#include "kctsdialog.h"
#include "IOspec.h"
#include "support.h"
#include "cstack.h"
#include "k_io.h"
#include "debug.h"

using namespace std;

CSerialComm kcts_serial;

extern bool test;
bool busy = false;

#define MAXTRIES 3
#define LOOPS 5

cStack commstack(1000);

unsigned char modes[5] = {'0','1','2','3','4'}; // AM, CW, LSB, USB, FM

bool startComms(const char *szPort, int baudrate)
{
	kcts_serial.Baud(9600);
	kcts_serial.Stopbits(1);
	kcts_serial.Device(szPort);
	if (kcts_serial.OpenPort() == false)
		return false;
	kcts_serial.Timeout(50); // msec timeout for read from rig
	return true;
}

bool sendCmd(string &str) {
	return sendCommand( (char*)str.c_str());
}

string retval;

bool sendCommand(char *str)
{
//	if (testing) {
//		retval = "TESTING";
//		return true;
//	}
	int len = str[0];
	int nret, loopcnt;
	bool ret = false;
	unsigned char *sendbuff = new unsigned char(len+2);
	unsigned char retbuff[3];

	pthread_mutex_lock(&mutex_serial);

// create command string
	sendbuff[0] = STX;
	for (int n = 1; n <= len; n++) sendbuff[n] = str[n];
	sendbuff[len+1] = ETX;
	len += 2;

	for (int i = 0; i < MAXTRIES; i++) {
		nret = kcts_serial.WriteBuffer ((const char *)sendbuff, len);
		if (nret != len)
			continue; // write error, retry
		loopcnt = 0;
		do  {
			memset(retbuff, 0, 3);
			nret = kcts_serial.ReadBuffer ((char *)retbuff, 1);
			if (retbuff[0] == 0xFF) { // 505 accepted the command
				retval = "OK";
				ret = true;
				goto cmddone;
			}
			if (retbuff[0] == 0xFE) { // 505 rejected the command
				retval = "REJ";
				goto cmddone;
				break;
			}
			commstack.push(retbuff[0]); // telemetry data
		} while (++loopcnt < LOOPS);
	}
	retval = "FAIL";
cmddone:
	delete [] sendbuff;
	pthread_mutex_unlock(&mutex_serial);
	return ret;
}

bool RequestData (char *cmd, unsigned char *buff, int nbr)
{
//	if (testing) {
//		retval = "TESTING";
//		return false;
//	}
	int len = cmd[0];
	int nret, loopcnt;
	bool ret = false;
	unsigned char *sendbuff = new unsigned char(len+2);
	unsigned char retbuff[3];
	char szTemp[10];

	pthread_mutex_lock(&mutex_serial);

// create command string
	sendbuff[0] = STX;
	for (int n = 1; n <= len; n++) sendbuff[n] = cmd[n];
	sendbuff[len+1] = ETX;
	len += 2;

	retval.clear();
	for (int i = 0; i < MAXTRIES; i++) {
		nret = kcts_serial.WriteBuffer ((const char *)sendbuff, len);
		if (nret != len)
			continue; // write error, retry
		loopcnt = 0;
		do  {
			memset(retbuff, 0, 3);
			nret = kcts_serial.ReadBuffer ((char *)retbuff, 1);
			if (retbuff[0] == 0xFD) { // kachina is sending the data
				kcts_serial.ReadBuffer ((char *)buff, nbr);
				for (int i = 0; i < nbr; i++) {
					if (i && !(i % 16)) retval.append("\n");
					snprintf(szTemp, sizeof(szTemp), "%02X ", buff[i]);
					retval.append(szTemp);
				}
				ret = true;
				goto reqdone;
			}
			if (retbuff[0] == 0xFE) { // 505 rejected the command
				retval = "REJ";
				goto reqdone;
			}
			commstack.push(retbuff[0]); // telemetry data
		} while (++loopcnt < LOOPS);
	}
	retval = "FAIL";
reqdone:
	delete [] sendbuff;
	pthread_mutex_unlock(&mutex_serial);
	return ret;
}

bool sendData (char *str, unsigned char *buff, int nbr)
{
	int len = 0;
	int cmdlen = str[0];
	int buffsize = cmdlen + 2 + nbr;
	unsigned char sendbuff[buffsize + 1];
//	unsigned char retbuff[3];
	
//	busy = true;
	memset(sendbuff, 0, buffsize+1);
// create command string	
	sendbuff[len++] = STX;
	for (int n = 1; n <= cmdlen; n++) sendbuff[len++] = str[n];
	sendbuff[len++] = ETX;

	for (int n = 0; n < nbr; n++)
		sendbuff[len++] = buff[n];

LOG_WARN("%s\n", str2hex(sendbuff, 2 + nbr + cmdlen));
return true;
/*
	char prtbuff[len * 3 + 2];
	char sData[4];
	strcpy(prtbuff,"<stx> ");
	snprintf(sData, sizeof(sData), "%c ", sendbuff[1]); 
	strcat(prtbuff, sData);
	for (int i = cmdlen + 2; i < len; i++) {
		snprintf(sData, sizeof(sData), "%02X ", sendbuff[i]);
		strcat(prtbuff, sData);
	}
	strcat(prtbuff, "<etx>\n");

	strcat(prtbuff,"\n");
	LOG_WARN("%s", prtbuff);

//	busy = false;

//	return true;

	int nret, loopcnt;
	char szVal[20];
	
	for (int i = 0; i < MAXTRIES; i++) {
		nret = kcts_serial.WriteBuffer ((char *)sendbuff, len); // write error, retry
		if (nret != len)
			continue;
		loopcnt = 0;
		do  {
			memset(retbuff, 0, 3);
			nret = kcts_serial.ReadBuffer ((char *)retbuff, 1);
			if (retbuff[0] == 0xFF) { // Kachina accepted the data
				LOG_WARN("%s","[FF]");
				busy = false;
				return true;
			}
			if (retbuff[0] == 0xFE) { // Kachina rejected the data
				LOG_WARN("%s", "[FE]");
				break;
			}
			if (test) {
				snprintf(szVal, sizeof(szVal), "[%02X]", retbuff[0]);
				LOG_WARN("%s", szVal);
			}
			commstack.push(retbuff[0]); // telemetry data
		} while (++loopcnt < LOOPS);		
	}
	LOG_ERROR("%s", "Failed\n");
//	busy = false;
	return false;
*/
}

bool setXcvrNOOP()
{
	return sendCommand(cmdK_NOOP);
}

