#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h> //标准IO库 输入输出出错(error)
#include <sys/mman.h>
#include <stdlib.h>

#define FB_PATH "/dev/fb0" //宏定义简单的字符替换 帧缓冲地址

/*全局变量--作用域整个文件*/
int *plcd = NULL;//指针
int fd_plcd = -1;//屏幕的文件描述符

/*函数声明--声明函数 函数定义在主函数后面*/
void Init_LCD(void);
void Unit_LCD(void);
void LCD_Draw_Point(int x,int y,int color);
void Draw_Matrix(int x,int y,int w,int h,int color);
int Bmp_display(char *bmp_file,int x0,int y0);

int getrand(){
	srand( (unsigned)time( NULL ) ); 
	return rand()%10+1;//rand()%100表示取100以内的随机数，即取了随机数后再对100取余  x=rand()%(Y-X+1)+X 
}
/*主函数*/
int main()
{
	//实现在屏幕上显示黄色的矩形
	//1.打开屏幕
	Init_LCD();
	//2.画矩形
	Draw_Matrix(50,50,600,300,0xFFFF00);
	sleep(3);//暂停 3s
	int i=1;
	while(1)
	{
		char PATH[64]={0};//存放图片的路径名
		sprintf(PATH,"%d.bmp",i++);//格式化输出到PATH所指向的地方
		Bmp_display(PATH,0,0);
		sleep(2);//每隔一秒显示一次
		//if(i==7)//控制循环播放的
		//	i=1;W
		i = getrand()
	}
	//3.关闭屏幕
	Unit_LCD();	
}

/*功能函数*/




/*初始化屏幕*/
void Init_LCD(void)
{
	fd_plcd = open(FB_PATH,O_RDWR);
	if(-1 == fd_plcd)
	{
		perror("open LCD fail");
		return ;
	}
	//映射
	plcd = mmap(NULL,480*800*4,PROT_READ|PROT_WRITE,MAP_SHARED,fd_plcd,0);
	if(plcd == MAP_FAILED)
	{
		perror("mmap fail");
		return ;
	}
}

/*解初始化屏幕--关闭屏幕 解除映射*/
void Unit_LCD(void)
{
	munmap(plcd,800*480*4);
	plcd = NULL;
	close(fd_plcd);
}

/*点亮屏幕x y处的像素点为color*/
void LCD_Draw_Point(int x,int y,int color)
{
    if(x>=0&&x<800&&y>=0&&y<480)
    {
        *(plcd+800*y+x)=color;
	}
}

/*在屏幕的x y处画一个宽为w 高为h 颜色为color的矩形*/
void Draw_Matrix(int x,int y,int w,int h,int color)
{
	int i,j;
	for(i=y;i<y+h;i++)//遍历行
	{
		for(j=x;j<x+w;j++)//遍历列
		{
			LCD_Draw_Point(j,i,color);
		}
	}
}

/*画一个圆*/
void Draw_circle()
{
}

/*解析显示一张bmp图片的*/
int Bmp_display(char *bmp_file,int x0,int y0)
{
	//1.打开bmp图片
	int fd = open(bmp_file,O_RDONLY);
	if(-1==fd)
	{
		printf("open %s fail\n",bmp_file);
		perror("--->");
		return -1;
	}
	//2.判断到底是不是一张bmp图片
	unsigned char buf[4];
	read(fd,buf,2);
	if(buf[0]!= 0x42 || buf[1]!= 0x4d)//若果不是B M
	{
		printf("NOT BMP\n");
		goto ERROR_END;
	}
	//3.获取bmp图片的属性 宽 高 色深
	int width,height;
	short depth;
	lseek(fd,0x12,SEEK_SET);
	read(fd,buf,4);
	
	width=(buf[3]<<24)|
		  (buf[2]<<16)|
		  (buf[1]<<8)|
		  (buf[0]);

	lseek(fd,0x16,SEEK_SET);
	read(fd,buf,4);
	
	height=(buf[3]<<24)|
		  (buf[2]<<16)|
		  (buf[1]<<8)|
		  (buf[0]);

	lseek(fd,0x1c,SEEK_SET);
	read(fd,buf,2);
	
	depth=(buf[1]<<8)|
		  (buf[0]);

	//只支持色深为24和32的
	if(!(depth == 24 || depth == 32))
	{
		printf("NOT Support!\n");
		goto ERROR_END;
	}
	printf("%s:%d*%d depth=%d\n",bmp_file,width,height,depth);
	//4.获取像素数组
	int line_valid_bytes = abs(width)*depth/8;//一行有效字节数
	int line_bytes;//一行总字节数=有效字节数+赖子数 
	int laizi = 0;
	
	if(line_valid_bytes%4)
	{
		laizi = 4-line_valid_bytes%4;
	}
	
	line_bytes = line_valid_bytes + laizi;
	
	int total_bytes = line_bytes*abs(height);//整个像素数组的大小

	//是不是需要一个地方来存放读出来的这个数组呢？？
	//unsigned char piexl[total_bytes];
	//可以向系统申请一块动态内存
	unsigned char *piexl = (unsigned char *)malloc(total_bytes);

	lseek(fd,54,SEEK_SET);
	read(fd,piexl,total_bytes);
	//5.在屏幕上一一显示这些像素点
	//读像素点字节--->ARGB--->在屏幕上进行显示
	unsigned char a,r,g,b;
	int color;
	int i = 0;

	int x,y;
	for(y=0;y<abs(height);y++)
	{
		for(x=0;x<abs(width);x++)
		{
			//a r g b 0xargb 小端模式  b g r a
			b = piexl[i++];
			g = piexl[i++];
			r = piexl[i++];
			if(depth == 32)
			{
				a = piexl[i++];
			}
			else
			{
				a = 0;//不透明
			}
			color=(a<<24)|(r<<16)|(g<<8)|(b);
			//printf("%x\n",color);
			//在屏幕对应的位置显示
			
			LCD_Draw_Point(width>0?x0+x:x0+abs(width)-x-1, 
							height>0?y0+abs(height)-y-1:y0+y,
							color);
		}
		//每一行的末尾 有可能填充几个赖子
		i += laizi;
	}
	

	free(piexl);
	close(fd);
	ERROR_END:
		close(fd);
		return -2;
}








