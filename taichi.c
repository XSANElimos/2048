#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <sys/mman.h>
#include<unistd.h>
#include<math.h> 
int *plcd = NULL;
#define WHITE 0x00FFFFFF 
#define BLAK 0x00000000 
void draw_point(int x, int y, int color)
{
	if (x >= 0 && x<800 && y >= 0 && y<480)
	{
		*(plcd + y * 800 + x) = color;
	
	}
}
 
void draw_circle(int x, int y,double r ,int color)
{
	if (x >= 0 && x<480 && y >= 0 && y<800)
	{
		for (double i = 0; i < 480; i++)
    {
		for (double j = 0; j < 800; j++)
			{
				double all=(i-x)*(i-x)+(j-y)*(j-y);
				double fc=sqrt(all);
				if(r>fc)
				{
						draw_point(j, i, color);
					//	printf("fc=%lf\n",fc);
				}
			}
	}
	
	}
}
 
void draw_circle_b(int x, int y,double r ,int color)
{
	if (x >= 0 && x<480 && y >= 0 && y<800)
	{
		for (double i = 0; i < 480; i++)
    {
		for (double j = 0; j < 800; j++)
			{
				if(i<x){
					double all=(i-x)*(i-x)+(j-y)*(j-y);
					double fc=sqrt(all);
					if(r>fc)
					{
						draw_point(j, i, color);
					//	printf("fc=%lf\n",fc);
					}					
				}
			}
	}
	
	}
}
 
void clear(int color)
{
	int x,y;
	for(y=0;y<480;y++)
	{
		for(x=0;x<800;x++)
		{
			draw_point(x,y,color);
		}
	}
}
 
int main()
{
	int lcd_fd = open("/dev/fb0",O_RDWR);
	if (lcd_fd == -1)
	{
		perror("open lcd fail");
	}
	plcd = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED,lcd_fd,0);
	if (plcd==NULL)
	{
		perror("mmao  fail");
	}
	int color = 0x0000FFFF;
	
    clear(0x00666666);
	draw_circle(240, 400,200, BLAK);
	draw_circle_b(240, 400,200, WHITE);
    draw_circle(240, 300,100, WHITE); 
    draw_circle(240, 500,100, BLAK); 
    draw_circle(240, 300,25, BLAK); 
    draw_circle(240, 500,25, WHITE); 
 
//	draw_circle(240, 400,50, color);  
	close(lcd_fd);
	munmap(plcd,800*480*4);
	return 0;
}