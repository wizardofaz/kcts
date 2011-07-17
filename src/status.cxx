/* -----------------------------------------------------------------------------
 * status structure / methods
 *
 * A part of "rig", a rig control program compatible with fldigi / xmlrpc i/o
 *
 * copyright Dave Freese 2009, w1hkj@w1hkj.com
 *
*/

#include <iostream>
#include <fstream>
#include <string>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Progress.H>

#include "kcts.h"
#include "IOspec.h"
#include "support.h"
#include "cstack.h"
#include "kcts_io.h"
#include "config.h"
#include "status.h"

struct XCVRSTATE xcvrState = { 
	0,		// MAIN_X
	0,		// MAIN_Y
	"",			// string ttyport
};

void XCVRSTATE::saveLastState()
{
	Fl_Preferences spref(homedir.c_str(), "w1hkj.com", PACKAGE_NAME);

	mainX = window->x();
	mainY = window->y();

	spref.set("version", PACKAGE_VERSION);
	spref.set("mainx", mainX);
	spref.set("mainy", mainY);
	spref.set("ttyport", ttyport.c_str());
}

void XCVRSTATE::loadLastState()
{
	Fl_Preferences spref(homedir.c_str(), "w1hkj.com", PACKAGE_NAME);

	if (spref.entryExists("version")) {

		char defbuffer[200];
//		int i = 0;

		spref.get("mainx", mainX, mainX);
		spref.get("mainy", mainY, mainY);
		spref.get("ttyport", defbuffer, "", 199);
		ttyport = defbuffer;
	}
}
