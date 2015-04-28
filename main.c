#include <opencv\highgui.h>
#include <opencv\cxcore.h>
#include <opencv\cv.h>
#include "typeSO.h"

#define IMGDATA(image,i,j,k) *((uchar *)&image->imageData[(i)*(image->widthStep) + (j)*(image->nChannels) + (k)])  //macro for data manipulation of image

void SO(IplImage* L, IplImage* R, IplImage * Disparity, int dmin, int dmax, float P1, float P2,int scale_factor, int type);
void init_xy(int*x, int*y, int * previous_x,int* previous_y, int width, int height,int typeSO);
float global_cost(IplImage *L,IplImage *R,int x, int y, int dmin, int dmax,float P1, float P2);
void SGM (IplImage* L, IplImage* R, IplImage * Disparity, int dmin, int dmax, float P1, float P2);
void SGM1 (IplImage* L, IplImage* R, IplImage * Disparity, int dmin, int dmax, float P1, float P2);


int main ()
{
	int a[2];
	IplImage * L=cvLoadImage("Tsukuba_L.png",1);
	IplImage * R=cvLoadImage("Tsukuba_R.png",1);
	IplImage * Disparity= cvCreateImage(cvSize(L->width,L->height),8,1);
	IplImage * Disparity1= cvCreateImage(cvSize(L->width,L->height),8,1);
	//SGM(L,R,Disparity,0,15,10,500);
	SO(L,R,Disparity1,0,15,10,500,16,LEFT_RIGHT);
	cvShowImage("l", L);
	cvShowImage("r", R);
	cvShowImage("d",Disparity1);

	a[0]=CV_IMWRITE_PNG_COMPRESSION;
	a[1]=9;
	//cvSaveImage("a.png",&Disparity,a);
	
	
	
	cvWaitKey(0);
	
}

float* global_costs(IplImage *L, IplImage* R, int x, int y, int dmin, int dmax, float P1, float P2, float* previous_global_costs)
{
	int d,i;
	float best_global_cost,temp_cost;
	float c; int a; //test
	float* global_costs=(float*)malloc(sizeof(float)*(dmax-dmin+1));



	for(d=dmin;d<=dmax;d++)
	{
		best_global_cost=INT_MAX*1.0;
		best_global_cost=previous_global_costs[d-dmin];
		if(d-dmin>=1)
		{
			temp_cost=previous_global_costs[d-dmin-1]+P1;
			if(temp_cost<best_global_cost)
				best_global_cost=temp_cost;

		}
		if(d<dmax)
		{
			temp_cost=previous_global_costs[d-dmin+1]+P1;
			if(temp_cost<best_global_cost)
				best_global_cost=temp_cost;
		}

		
		for(i=dmin; i<=d-2; i++)
		{
			float temp_global_cost=previous_global_costs[i-dmin]+P2;
			if(temp_global_cost<best_global_cost)
			{
				
				best_global_cost=temp_global_cost;
			}
		}
			
		for(i=d+2; i<=dmax; i++)
		{
			float temp_global_cost=previous_global_costs[i-dmin]+P2;
			if(temp_global_cost<best_global_cost)
			{
				
				best_global_cost=temp_global_cost;
			}
		}
		a=dmax-dmin;
		c=best_cost(previous_global_costs,a);
		global_costs[d]=best_global_cost+cost(L,R,x,y,d)-best_cost(previous_global_costs,a);
		//printf("%d ",cost(L,R,x,y,index));
		
	}
	return global_costs;
}


void SGM (IplImage* L, IplImage* R, IplImage * Disparity, int dmin, int dmax, float P1, float P2)
{
	int x,y,d;
	float* previous_global_costs;
	float* current_global_costs;//=(float*)malloc(sizeof(float)*(dmax-dmin+1));;
	
	for(x=0; x<L->height;x++)
	{
		previous_global_costs=(float*)calloc(sizeof(float),(dmax-dmin+1));
		for(y=0; y<L->width;y++)
		{			
			current_global_costs=global_costs(L,R,x,y,dmin,dmax,P1,P2,previous_global_costs);
			free(previous_global_costs);
			IMGDATA(Disparity,x,y,0)=best_disparity(current_global_costs,dmax-dmin)*16;
			
			previous_global_costs=current_global_costs;
		}
	}
	
}

float cost(IplImage* L, IplImage* R, int x,int y, int d)
{
	int x_R=x-d;
	if(x_R<0)
		x_R=0;
	
	return abs((CV_IMAGE_ELEM(L,unsigned char,y,x*3)-CV_IMAGE_ELEM(R,unsigned char,y,x_R*3))
		+(CV_IMAGE_ELEM(L,unsigned char,y,x*3+1)-CV_IMAGE_ELEM(R,unsigned char,y,x_R*3+1))
		+(CV_IMAGE_ELEM(L,unsigned char,y,x*3+2)-CV_IMAGE_ELEM(R,unsigned char,y,x_R*3+2))
		/3
		);
}

int best_disparity(float global_costs[],int size)
{
	int best_disparity,d,current_global_cost;
	float best_global_cost=INT_MAX*1.0;
	for(d=0;d<=size;d++)
	{
		current_global_cost=global_costs[d];
		if(current_global_cost<best_global_cost)
		{
			best_disparity=d;
			best_global_cost=current_global_cost;
		}
	}
	return best_disparity;
}

float best_cost(float global_costs[], int size)
{
	int index;
	float current_global_cost;
	float best_global_cost=global_costs[0];

	for(index=0;index<=size;index++)
	{
		current_global_cost=global_costs[index];
		if(current_global_cost<best_global_cost)
			{
				best_global_cost=current_global_cost;
		}
	}
	return best_global_cost;
}


void SO(IplImage* L, IplImage* R, IplImage * Disparity, int dmin, int dmax, float P1, float P2,int scale_factor, int type)
{
	int test;
	float* previous_global_costs;
	float* current_global_costs;
	int height=L->height;
	int width=L->width;
	int x,y,previous_x, previous_y;
	previous_global_costs=(float*)calloc(sizeof(float),(dmax-dmin+1));

	init_xy(&x,&y,&previous_x,&previous_y,width,height, type);
	
	printf("%d %d %d %d", x,y,previous_x,previous_y);


	while (scan_xy(&x,&y, previous_x, previous_y,width,height,type))
	{
		current_global_costs=global_costs(L,R,x,y,dmin,dmax,P1,P2,previous_global_costs);
		test=best_disparity(current_global_costs,dmax-dmin)*scale_factor;
		CV_IMAGE_ELEM(Disparity,unsigned char,y,x)=best_disparity(current_global_costs,dmax-dmin)*scale_factor;
		//IMGDATA(Disparity,x,y,0)=best_disparity(current_global_costs,dmax-dmin)*scale_factor;

		//free(previous_global_costs);
		previous_global_costs=current_global_costs;
		
		previous_x=x;
		previous_y=y;
	}
}

void init_xy(int*x, int*y, int * previous_x,int* previous_y, int width, int height,int typeSO)
{
	if((typeSO==LEFT_RIGHT) || typeSO==UPLEFT_DOWNRIGHT || typeSO==UP_DOWN)
	{
		*previous_x=-1;
		*previous_y=0;
		*x=0;
		*y=0;
	}

	if(typeSO==RIGHT_LEFT || typeSO==UPRIGHT_DOWNLEFT)
	{
		*x=width-1;
		*y=0;
	}

	if(typeSO==DOWNRIGHT_UPLEFT || typeSO==DOWN_UP)
	{
		*x=width-1;
		*y=height-1;
	}

	if(typeSO==DOWNLEFT_UPRIGHT)
	{
		*x=0;
		*y=height - 1;
	}
}

int scan_xy(int* x,int* y,int previous_x,int  previous_y, int width,int height, int typeSO)
{
	if(typeSO==LEFT_RIGHT)
	{
		if(previous_x>=width)
		{
			*y=previous_y+1;
			*x=0;
		}
		else
			*x=previous_x+1;
		if(*y>=height && *x>=width)
			return 0;
		else
			return 1;
	}

}