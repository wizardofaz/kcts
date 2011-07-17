// XYplot.h
#ifndef XYplot_H
#define XYplot_H

#if defined(WIN32) && !defined(__CYGWIN__)
#  include <direct.h>
#else
#  include <unistd.h>
#endif
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Counter.H>
#include <FL/fl_draw.H>
#include <FL/math.h>
#include <stdio.h>
#include <stdlib.h>

class XYplot : public Fl_Widget {
  double xmin, xmax;
  double ymin, ymax;
  double *xarray;
  double *yarray;
  int  arraysize;
  void draw();
public:
  XYplot(int X,int Y,int W,int H, const char *lbl = "") : Fl_Widget(X,Y,W,H,lbl)
  {
//    box(FL_DOWN_BOX);
    xmin = 0.0; xmax = 100.0;
	ymin = 0.0; ymax = 100.0;
	xarray = yarray = 0;
	arraysize = 0;
  }
  ~XYplot() {
	  if (xarray) delete [] xarray;
	  if (yarray) delete [] yarray;
  }
  void plotXY(double *xvals, double *yvals, int numvals);
  void xMin(double xm) { xmin = xm; }
  void xMax(double xM) { xmax = xM; }
  void yMin(double ym) { ymin = ym; }
  void yMax(double yM) { ymax = yM; }
  void end() {};
};

#endif
