#ifndef _KACHINA_IO_H
#define _KACHINA_IO_H

#include <string>
#include "serialcomm.h"
#include "cstack.h"

extern CSerialComm kcts_serial;

extern std::string retval;

extern bool startComms(const char *, int);

extern bool sendCommand(char *str);
extern bool sendCmd(std::string &str);
extern bool RequestData (char *cmd, unsigned char *buff, int nbr);
extern bool sendData (char *str, unsigned char *buff, int nbr);

extern cStack commstack;

extern bool setXcvrNOOP();

#endif
