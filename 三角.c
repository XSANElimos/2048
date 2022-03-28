#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>标准IO库 输入输出出错(error)

#define FB_PATH devfb0 宏定义简单的字符替换  
#define BLACK 0x000000
#define RED 0xFF0000
#define YELLOW 0xEEEE00

int main()
{

	1.打开屏幕
	int fb = open(FB_PATH,O_RDWR);
	if(-1 == fb)
	{
		perror(open LCD fail);
		return -1;
	}
	2.写
	int color[480][800];
	int i,j,length=800;

	for(i=0;i480;i++)
	{
		for(j=0;j800;j++)
		{
			color[i][j] = RED;
		}
	}

	for(i=0;i240;i++)
	{
		for(j=0;jlength;j++)
		{
			color[i][j] = YELLOW;
		}
		length--;
	}
	
	for(i=240;i480;i++)
	{
		for(j=0;jlength;j++)
		{
			color[i][j] = YELLOW;
		}
		length++;
	}
	write(fb,color,4808004);
	关闭屏幕
	close(fb);
}

