#ifndef K_IO_H
#define K_IO_H

#include <string>

#include "cstack.h"
#include "serialcomm.h"

extern std::string retval;

extern CSerialComm kcts_serial;

extern bool startComms(const char *, int);

extern void showRigSerial( char *str);

extern bool sendCommand(char *str);
extern bool RequestData (char *cmd, unsigned char *buff, int nbr);
extern bool sendData (char *str, unsigned char *buff, int nbr);

extern cStack commstack;

#endif
