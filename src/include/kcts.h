#ifndef _kcts_H
#define _kcts_H

//#define DEBUG 1

#include <string>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Enumerations.H>
#include <sys/types.h>
#include <pthread.h>

#ifndef WIN32
#include <unistd.h>
#include <pwd.h>
#endif

#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/fl_draw.H>

#include "kctsdialog.h"
#include "kcts_io.h"
#include "serialcomm.h"

using namespace std;

extern pthread_t *watchdog_thread;
extern pthread_t *serial_thread;
extern pthread_t *telemetry_thread;

extern pthread_mutex_t mutex_watchdog;
extern pthread_mutex_t mutex_serial;
extern pthread_mutex_t mutex_telemetry;

extern Fl_Double_Window *window;
extern string homedir;
extern char defFileName[200];

extern void about();
extern void visit_URL(void* arg);

#endif
