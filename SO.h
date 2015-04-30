#include <opencv\highgui.h>
#include <opencv\cxcore.h>
#include <opencv\cv.h>
#include "typeSO.h"


float* global_costs(IplImage *L, IplImage* R, int x, int y, int dmin, int dmax, float P1, float P2, float* previous_global_costs);

float cost(IplImage* L, IplImage* R, int x,int y, int d);

int best_disparity(float global_costs[],int size);

float best_cost(float global_costs[], int size);

void SO(IplImage* L, IplImage* R, IplImage * Disparity, int dmin, int dmax, float P1, float P2,int scale_factor, int type);

void init_xy(int*x, int*y, int * previous_x,int* previous_y, int width, int height,int typeSO);

int scan_xy(int* x,int* y,int previous_x,int  previous_y, int width,int height, float* previous_global_costs,int size, int typeSO);

