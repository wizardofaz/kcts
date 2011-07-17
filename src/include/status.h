#ifndef _status_H
#define _status_H

#include <string>
#include <FL/Fl.H>
#include <FL/Enumerations.H>

using namespace std;

// kcts parameters & state variables
struct XCVRSTATE {
	int		mainX;
	int		mainY;
	string	ttyport;

	void saveLastState();
	void loadLastState();
};

extern struct XCVRSTATE xcvrState;

#endif
