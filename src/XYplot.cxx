// Class definition for Log/Log plot
// db vs freq

#include "XYplot.h"

void XYplot::draw()
{
	int bw = 4;
	int X = x() + bw, Y = y() + bw, W = w() - 2*bw, H = h() - 2*bw;
	double xscale, yscale;
	int x1, y1, x2, y2;
	
	fl_draw_box(FL_DOWN_BOX, x(), y(), w(), h(), FL_WHITE);
	
	fl_clip(X,Y,W,H); //x(),y(),w(),h());
//	fl_color(FL_WHITE);
//	fl_rectf(X,Y,W,H);
	fl_push_matrix();
	fl_color(FL_BLUE);

	xscale = xmax - xmin;
	if (xscale == 0.0) xscale = 1.0e-8;
	yscale = ymax - ymin;
	if (yscale == 0.0) yscale = 1.0e-8;
	xscale = W / xscale;
	yscale = -H / yscale;
	
	if (arraysize) {
		x1 = X + (int)((xarray[0] - xmin) * xscale);
		y1 = Y + H + (int)((yarray[0] - ymin) * yscale);
		for (int i = 1; i < arraysize; i++) {
			x2 = X + (int)((xarray[i] - xmin) * xscale);
			y2 = Y + H + (int)((yarray[i] - ymin) * yscale);
			fl_line( x1, y1, x2, y2 );
			x1 = x2; y1 = y2;
		}
	}

  fl_pop_matrix();
  fl_pop_clip();
}

void XYplot::plotXY(double *xvals, double *yvals, int num)
{
	if (xarray) delete [] xarray;
	if (yarray) delete [] yarray;
	xarray = new double[num];
	yarray = new double[num];
	arraysize = num;
	for (int i = 0; i < num; i++) {
		xarray[i] = *xvals++;
		yarray[i] = *yvals++;
	}
}
