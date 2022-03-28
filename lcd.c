#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h> //标准IO库 输入输出出错(error)

#define FB_PATH "/dev/fb0" //宏定义简单的字符替换  
#define BLACK 0x000000
#define RED 0xFF0000
#define YELLOW 0xEEEE00

int* Draw_circle(int x , int y , int r , unsigned int color){
	int color[480][800];


	return color;
}


int main()
{

	//1.打开屏幕
	int fb = open(FB_PATH,O_RDWR);
	if(-1 == fb)
	{
		perror("open LCD fail");
		return -1;
	}
	//2.写
	color = Drawdraw_circle(240,400,20,YELLOW);
	write(fb,color,480*800*4);
	//关闭屏幕
	close(fb);
}


