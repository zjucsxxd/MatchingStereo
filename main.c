#include <opencv\highgui.h>
#include <opencv\cxcore.h>
#include <opencv\cv.h>

#define IMGDATA(image,i,j,k) *((uchar *)&image->imageData[(i)*(image->widthStep) + (j)*(image->nChannels) - (k)])  //macro for data manipulation of image

float global_cost(IplImage *L,IplImage *R,int x, int y, int dmin, int dmax,float P1, float P2);
void SGM (IplImage* L, IplImage* R, IplImage * Disparity, int dmin, int dmax, float P1, float P2);


int main ()
{
	IplImage * L=cvLoadImage("Cones_L.png",1);
	IplImage * R=cvLoadImage("Cones_R.png",1);
	IplImage * Disparity= cvCreateImage(cvSize(L->width,L->height),8,1);
	SGM(L,R,Disparity,0,4,1,10);
	cvShowImage("l", L);
	cvShowImage("r", R);
	cvShowImage("d",Disparity);
	
	
	cvWaitKey(0);
	
}

/*IplImage* SGM_L_to_R (IplImage* L, IplImage* R, int dmin, int dmax, float P1, float P2, IplImage* Disp)
{
	CvSize size= cvSize(L->width,L->height);
	IplImage *test=cvCreateImage(size,L->depth,1);//test
	

	int x,y,d;

	//memset(test->imageData, 255, test->widthStep*test->height);
	for(y=0; y<=L->height;y++)
	{
		for(x=0; x<=L->width;x++)
		{
			
			IMGDATA(test,x,y,0)=cost(L,R,x,y,dmin,dmax,P1,P2); //test
		}
	}
	cvShowImage("test",test);
	return (test);//test
	//cvWaitKey(0);//test
}*/





float* global_costs(IplImage *L, IplImage* R, int x, int y, int dmin, int dmax, float P1, float P2, float* previous_global_costs)
{
	int d,i;
	float best_global_cost,temp_cost;
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
		global_costs[d]=best_global_cost+cost(L,R,x,y,d);
		//printf("%d ",cost(L,R,x,y,index));
		
	}
	return global_costs;
}


void SGM (IplImage* L, IplImage* R, IplImage * Disparity, int dmin, int dmax, float P1, float P2)
{
	int x,y,d;
	float* previous_global_costs;
	float* current_global_costs;//=(float*)malloc(sizeof(float)*(dmax-dmin+1));;
	previous_global_costs=(float*)malloc(sizeof(float)*(dmax-dmin+1));
	for(d=0; d<=dmax-dmin; d++)
	{
		previous_global_costs[d]=0;
	}
	for(x=0; x<L->height;x++)
	{
		for(y=0; y<L->width;y++)
		{			
			current_global_costs=global_costs(L,R,x,y,dmin,dmax,P1,P2,previous_global_costs);
			IMGDATA(Disparity,x,y,0)=best_disparity(current_global_costs,dmax-dmin)*4;
			free(previous_global_costs);
			previous_global_costs=current_global_costs;
		}
	}
	
}

float cost(IplImage* L, IplImage* R, int x,int y, int d)
{
	int y_R=y-d;
	if(y_R<0)
		y_R=0;
	return (IMGDATA(L,x,y,0)-IMGDATA(R,x,y_R,0));
}

int best_disparity(float global_cost[],int size)
{
	int best_disparity,d,current_global_cost;
	float best_global_cost=INT_MAX*1.0;
	for(d=0;d<=size;d++)
	{
		current_global_cost=global_cost[d];
		if(current_global_cost<best_global_cost)
		{
			best_disparity=d;
			best_global_cost=current_global_cost;
		}
	}
	return best_disparity;
}