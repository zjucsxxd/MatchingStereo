
#include "SO.h"


float* global_costs(IplImage *L, IplImage* R, int x, int y, int dmin, int dmax, float P1, float P2, float* previous_global_costs)
{
	int d,i;
	float best_global_cost,temp_cost;
	float* global_costs=(float*)malloc(sizeof(float)*(dmax-dmin));



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
		global_costs[d]=best_global_cost+cost(L,R,x,y,d)-best_cost(previous_global_costs,dmax-dmin);
		//printf("%d ",cost(L,R,x,y,index));
		
	}
	return global_costs;
}



float cost(IplImage* L, IplImage* R, int x,int y, int d)
{
	int x_R=x-d;
	int y_R=y;
	if(x_R<0)
		x_R=0;
	
	return abs((CV_IMAGE_ELEM(L,unsigned char,y,x*3)-CV_IMAGE_ELEM(R,unsigned char,y_R,x_R*3))
		+(CV_IMAGE_ELEM(L,unsigned char,y,x*3+1)-CV_IMAGE_ELEM(R,unsigned char,y_R,x_R*3+1))
		+(CV_IMAGE_ELEM(L,unsigned char,y,x*3+2)-CV_IMAGE_ELEM(R,unsigned char,y_R,x_R*3+2))
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
	
	while (scan_xy(&x,&y, previous_x, previous_y,width,height,previous_global_costs,dmax-dmin+1, type))
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
	if((typeSO==LEFT_RIGHT)  || typeSO==UP_DOWN)
	{
		*previous_x=-1;
		*previous_y=-1;
		*x=0;
		*y=0;
	}

	if(typeSO==RIGHT_LEFT)
	{
		*x=width-1;
		*y=0;
		*previous_x=width;
		*previous_y=-1;
	}

	if(typeSO==DOWNRIGHT_UPLEFT)
	{
		*x=width-1;
		*y=height-1;
		*previous_x=height;
		*previous_y=width;
	}

	if(typeSO==DOWN_UP || typeSO==UPLEFT_DOWNRIGHT)
	{
		*x=0;
		*y=height - 1;
		*previous_x=-1;
		*previous_y=height;
	}
	if(typeSO==UPLEFT_DOWNRIGHT)
	{
		*previous_x=1;
	}

	if(typeSO==DOWNRIGHT_UPLEFT)
	{
		*x=width-1;
		*y=0;
		*previous_y=1;
		*previous_x=width-1;
	}

	if(typeSO==DOWNLEFT_UPRIGHT)
	{
		*x=0;
		*y=0;
		*previous_x=-1;
		*previous_y=-1;
	}

	if(typeSO==UPRIGHT_DOWNLEFT)
	{
		*x=width-1;
		*y=height-1;
		*previous_x=width;
		*previous_y=height;
	}
}

int scan_xy(int* x,int* y,int previous_x,int  previous_y, int width,int height, float* previous_global_costs,int size, int typeSO)
{
	
	if(typeSO==LEFT_RIGHT)
	{
		if(previous_x>=width)
		{
			*y=previous_y+1;
			*x=0;
			memset(previous_global_costs,0,size);
		}
		else
			*x=previous_x+1;
		if(*y>=height && *x>=width)
			return 0;
		else
			return 1;
	}

	if(typeSO==RIGHT_LEFT)
	{
		if(previous_x<=0)
		{
			*y=previous_y+1;
			*x=width-1;
			memset(previous_global_costs,0,size);
			//previous_global_costs=(float*)calloc(sizeof(float),size);
		}
		else
			*x=previous_x-1;
		if(*y>=height && *x<=0)
			return 0;
		else
			return 1;
	}

	if(typeSO==UP_DOWN)
	{
		if(previous_y>=height)
		{
			*x=previous_x+1;
			*y=0;
			memset(previous_global_costs,0,size);
			//previous_global_costs=(float*)calloc(sizeof(float),size);
		}
		else
			*y=previous_y+1;
		if(*x>=width && *y>=height)
			return 0;
		else
			return 1;
	}

	if(typeSO==DOWN_UP)
	{
		if(previous_y<=0)
		{
			*x=previous_x+1;
			*y=height-1;
			memset(previous_global_costs,0,size);
			//previous_global_costs=(float*)calloc(sizeof(float),size);
		}
		else
			*y=previous_y-1;
		if(*x>=width && *y<=0)
			return 0;
		else
			return 1;
	}

	if(typeSO==UPLEFT_DOWNRIGHT)
	{
		if(*x<*y)
		{
			if(previous_y>=height-1)
			{
				*x=0;
				*y=previous_y-previous_x-1;
				memset(previous_global_costs,0,size);
				//previous_global_costs=(float*)calloc(sizeof(float),size);
			}
			else
			{
				*x=previous_x+1;
				*y=previous_y+1;
			}
		}
		else
		if(*x==*y)
		{
			if(previous_y>=height-1)
			{
				*x=1;
				*y=0;
				memset(previous_global_costs,0,size);
				//previous_global_costs=(float*)calloc(sizeof(float),size);
			}
			else
			{
				*x=previous_x+1;
				*y=previous_y+1;
			}
		}
		else
			if(*x>*y)
			{
				if(previous_x<width-1)
				{
					if(previous_y>=height-1)
					{
						*x=previous_x-previous_y+1;
						*y=0;
						memset(previous_global_costs,0,size);
						//previous_global_costs=(float*)calloc(sizeof(float),size);
					}
					else
					{
						*x=previous_x+1;
						*y=previous_y+1;
					}
				}
				else
				{
					*x=previous_x-previous_y+1;
					*y=0;
					memset(previous_global_costs,0,size);
					//previous_global_costs=(float*)calloc(sizeof(float),size);
				}

			}

		if(previous_x>=(width)-1 && previous_y<=0)
			return 0;
		else
			return 1;
	}

	if(typeSO==DOWNRIGHT_UPLEFT)
	{
		if(*x>=*y)
		{
			if(previous_y<=0)
			{
				if(previous_x>width-height-1)
				{
					*x=width-1;
					*y=width-previous_x;
					memset(previous_global_costs,0,size);
					//previous_global_costs=(float*)calloc(sizeof(float),size);
				}
				else
				{
					*x=previous_x+height-2;
					*y=height-1;
					memset(previous_global_costs,0,size);
					//previous_global_costs=(float*)calloc(sizeof(float),size);
				}

			}
			else
			{
				*x=previous_x-1;
				*y=previous_y-1;
			}
		}
		else
		{
			if(previous_x<=0)
			{
				*x=height-2-previous_y;
				*y=height-1;
				memset(previous_global_costs,0,size);
				//previous_global_costs=(float*)calloc(sizeof(float),size);
			}
			else
			{
				*x=previous_x-1;
				*y=previous_y-1;
			}
		}

		if(previous_y>=(height)-1 && previous_x<=0)
			return 0;
		else
			return 1;
	}

	if(typeSO==DOWNLEFT_UPRIGHT)
	{
		
		if(*y<=0)
		{
			if(*x<height-1)
			{
				*x=0;
				*y=previous_x+1;
				memset(previous_global_costs,0,size);
			}
			else
			{
				*x=previous_x-(height)+2;
				*y=height-1;
				memset(previous_global_costs,0,size);
			}
		}
		else
		{
			if(*x>=width-1)
			{
				*y=height-1;
				*x=previous_y+(width-height)+1;
				memset(previous_global_costs,0,size);
			}
			else
			{
				*x=previous_x+1;
				*y=previous_y-1;
			}
		}

		if(previous_x>=width-1 && previous_y>=height-1)
			return 0;
		else
			return 1;
	}

	if(typeSO==UPRIGHT_DOWNLEFT)
	{
		if(*y>=height-1)
		{
			if(*x>(width-height))
			{
				*x=width-1;
				*y=previous_x-(width-height)-1;
				memset(previous_global_costs,0,size);
			}
			else
			{
				*x=previous_x+(height)-2;
				*y=0;
				memset(previous_global_costs,0,size);
			}
		}
		else
		{
			if(*x<=0)
			{
				*y=0;
				*x=previous_y-1;
				memset(previous_global_costs,0,size);
			}
			else
			{
				*x=previous_x-1;
				*y=previous_y+1;
			}
		}

		if(previous_x<=0 && previous_y<=0)
			return 0;
		else
			return 1;
	}
}