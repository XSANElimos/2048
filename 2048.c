/*TODO
[视图]
1. 底层显示实现[已完成]，底层触摸实现[已完成]
2. 显示开始界面（标题+按钮）[已完成] 显示数字矩阵 显示游戏界面按钮 显示菜单界面（提示+按钮） 显示积分榜（显示+返回主界面） 

[机制]
1. 按键响应
2. 切换场景 主界面->游戏界面->菜单界面->主界面->积分榜
3. 定位数字矩阵位置
4. 矩阵变换逻辑：初始化新矩阵 上下左右 合并数字 对比操作(保存旧数组 对比新数组) 输赢判断 随机产数 获取空位 道具使用（CD 与 作用）
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h> //标准IO库 输入输出出错(error)
#include <sys/mman.h>
#include <stdlib.h>
#include <time.h>
#include <linux/input.h>//输入事件 触摸屏

#define FB_PATH "/dev/fb0" //宏定义简单的字符替换 帧缓冲地址
#define NOOPTION 0
#define UP 1
#define LEFT 2
#define DOWN 3
#define RIGHT 4
#define STARTGAME 5
#define CONTINUEGAME 6
#define MENU 7
#define RECORD 8
#define RETURNMAIN 9
#define EXIT 10
#define BOOMER 11
//0非按钮 1上 2左 3下 4右 5开始游戏 6继续游戏 7菜单 8计分板 9返回主页

/*全局变量--作用域整个文件*/
int *plcd = NULL;//指针
int fd_plcd = -1;//屏幕的文件描述符
int touch_x=-1,touch_y=-1;//-1 -1在屏幕不存在
int isnewgame = 1; //1 新游戏 0 非新游戏
int userbehavior = 0; //用户操作监视
int emptylist[64] = {0};
int instart = 1 , ingame = 0 , inrecord = 0 ,inprogram = 1 , inmenu = 0;
int GameNum=0; //游戏次数
int Score[5]={4096,2048,1096,512,256}; //得分表
int matrix[4][4]={ //棋盘矩阵
  	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0
};


/*图片资源信息*/
char Pic_StartScreen[32]={"resource/pic/startscreen.bmp"};
char Pic_BackGround[32]={"resource/pic/background.bmp"};
char Pic_title[32]={"resource/pic/title.bmp"};
char Pic_Victory[32]={"resource/pic/victory.bmp"};
char Pic_Lose[32]={"resource/pic/lose.bmp"};
char Pic_MenuBackGroud[32]={"resource/pic/menu_bg.bmp"};

char Btn_Down[32]={"resource/icon/down.bmp"};
char Btn_Left[32]={"resource/icon/left.bmp"};
char Btn_Right[32]={"resource/icon/right.bmp"};
char Btn_Up[32]={"resource/icon/up.bmp"};

char Btn_NewGame[32]={"resource/icon/newgame.bmp"};
char Btn_Restart[32]={"resource/icon/restartgame.bmp"};
char Btn_Continue[32]={"resource/icon/continue.bmp"};
char Btn_Record[32]={"resource/icon/record.bmp"};
char Btn_Setting[32]={"resource/icon/setting.bmp"};
char Btn_Exit[32]={"resource/icon/exit.bmp"};
char Btn_Return[32]={"resource/icon/returnmain.bmp"};
char Btn_Item_Boomer[32]={"resource/icon/item_boomer.bmp"};
char Btn_Menu[32]={"resource/icon/menu.bmp"};


/*函数声明--声明函数 函数定义在主函数后面
[视图]
1. 底层显示实现[已完成]，底层触摸实现[已完成]
2. 显示开始界面（标题+按钮） 显示数字矩阵 显示游戏界面按钮 显示菜单界面（提示+按钮） 显示积分榜（显示+返回主界面） 

[机制]
1. 按键响应
2. 切换场景 主界面->游戏界面->菜单界面->主界面->积分榜
3. 定位数字矩阵位置
4. 矩阵变换逻辑：初始化新矩阵 上下左右 合并数字 对比操作(保存旧数组 对比新数组) 输赢判断 随机产数 获取空位 道具使用（CD 与 作用）
*/
void Init_LCD(void); //底层显示实现
void Unit_LCD(void); //底层显示实现
void LCD_Draw_Point(int x,int y,int color); //底层显示实现
void Draw_Matrix(int x,int y,int w,int h,int color); //底层显示实现
int Bmp_display(char *bmp_file,int x0,int y0); //底层显示实现
void get_user_touch(void); //底层触摸实现

void Disp_startscreen(int State);//显示开始界面（标题+按钮）
void Disp_gamepadui(void);  //显示游戏界面按钮
void Disp_nummatrix(void);  //显示数字矩阵
void Disp_menu(void);		//显示菜单
void Disp_record(void); 	//显示计分板

int Get_Zero_num(void);
void Init_Matrix(); // 重置矩阵
void Save_array(int a[][4],int n); //保存矩阵
int getmatrix_px(int i);



int ifbutton(int state , int x ,int y); //判断触摸行为
int ifGameOver();
int ischange(int a[][4]);
int getrand(int x);

void fun_up();
void fun_left();
void fun_down();
void fun_right();
void fun_useboomer();
void fun_startnewgame();
void fun_continuegame();
void fun_openmenu();
void fun_openrecord();
void fun_openmain();


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

/*绘制方块*/
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

/*解析显示一张bmp图片*/
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
	//printf("%s:%d*%d depth=%d\n",bmp_file,width,height,depth);
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

/*获取二维数组有多少个0元素*/
int Get_Zero_num(void)
{
  int i,j,count=0;
  for(i=0;i<4;i++)//遍历二维数组的每一个元素
  {
    for(j=0;j<4;j++)
    {
      if(matrix[i][j]==0)
      {
        count++;
      }
    }
  }
  return count;
}

/*随机位置填充随机数字*/
int fillempty(void)
{
  //随机位置
  int pos_k = rand()%Get_Zero_num()+1;//1~K
  int i,j,count=0;
  int num[] = {2,2,2,2,4,4,8};//模拟概率
  //遍历到填充位置
  for(i=0;i<4;i++)//遍历二维数组的每一个为0元素
  {
    for(j=0;j<4;j++)
    {
      if(matrix[i][j]==0)
      {
        count++;
        if(count == pos_k)
        {
          //填充
		  matrix[i][j]=num[rand()%7];
		  return 0;
        }
      }
    }
  }
}

/*保存矩阵*/
void Save_array(int a[][4],int n)//int a[] int[4] a[] ==>int a[][4]
{
	int i,j;
	for(i=0;i<n;i++)	
		for(j=0;j<4;j++)
			a[i][j]=matrix[i][j];
}

//返回值 有改变则为1 没有改变则为0
int ischange(int a[][4]){
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++)
			if(a[i][j]!=matrix[i][j])
				return 1;		
	return 0;
}


/*获取用户在触摸屏上点击的坐标*/
void get_user_touch(void)
{
	//1.打开触摸屏
	int fd = open("/dev/input/event0",O_RDONLY);//触摸屏属于输出设备 只读
	if(-1 == fd)
	{
		perror("open touch_lcd fail");
		return ;
	}

	//2.获取用户的输入
	struct input_event ev;
	int flag_x=0,flag_y=0;
	while(1)//为什么是while(1)的死循环 因为用户什么时候输入你不知道 只能轮询
	{

		int res = read(fd,&ev,sizeof(ev));
		if(res!=sizeof(ev))//如果说读出来没有该结构体大小 那么就是读取失败 再继续读
			continue;
		if(ev.type == EV_ABS&&ev.code == ABS_X)//获取X轴坐标
		{
			touch_x = ev.value;//
			flag_x = 1;
			if(flag_y)
				break;
		}
		if(ev.type == EV_ABS&&ev.code == ABS_Y)//获取Y轴坐标
		{

			touch_y = ev.value;
			flag_y = 1;
			if(flag_x)
				break;
		}
		//获取 了XY则退出该死循环
	}
	
	//3.关闭
	close(fd);
}

/*绘制开始界面 
State 状态
0 新游戏
1 有存档记录，可继续游戏
*/
void Disp_startscreen(int State){

	// 背景图
	Bmp_display(Pic_StartScreen,0,0);
	// 信息
	Bmp_display(Pic_title,190,100);
	// 按钮 计分板
	Bmp_display(Btn_Record,150,410);

	// 按钮 开始/继续游戏
	switch(State){
		case 0:
		Bmp_display(Btn_NewGame,350,410);
		break;
		case 1:
		Bmp_display(Btn_Continue,350,410);
		break;
	}
	// 按钮 退出游戏
	Bmp_display(Btn_Exit,550,410);

}

/*绘制游戏UI*/
void Disp_gamepadui(void){
	Bmp_display(Pic_BackGround,0,0);  // 背景
	Draw_Matrix(15,15,465,465,0x002060); //矩阵包裹
	Bmp_display(Btn_Up,600,280);  // 上
	Bmp_display(Btn_Left,510,370); // 左
	Bmp_display(Btn_Down,600,370);  // 下
	Bmp_display(Btn_Right,690,370);  // 右
	Bmp_display(Btn_Item_Boomer,480,40); //炸弹道具
	Bmp_display(Btn_Menu,480,120); // 菜单按钮
}

/*绘制矩阵*/
void Disp_nummatrix(void){
	char bmp_name[64]={0};
	for(int i = 0 ; i < 4 ; i++){
		for(int j = 0 ; j < 4 ; j++){
			if (matrix[i][j]){
				memset(bmp_name,0,64);//将数组清空
				sprintf(bmp_name,"resource//digit//digit_%d.bmp",matrix[i][j]);//格式化输出到PATH所指向的地方
				Bmp_display(bmp_name,getmatrix_px(i),getmatrix_px(j));
			}else{
				Draw_Matrix(getmatrix_px(i),getmatrix_px(j),100,100,0x002060);
			}
			
		}
	}
}

/*绘制菜单*/
void Disp_menu(void){
	//Draw_Matrix(250,40,300,400,0x0525AC);
	Bmp_display(Pic_MenuBackGroud,250,40);
	Bmp_display(Btn_Continue,345,70);
	Bmp_display(Btn_Restart,325,140);
	Bmp_display(Btn_Record,345,210);
	Bmp_display(Btn_Exit,345,280);
}

/*绘制计分板*/
void Disp_record(void){
	char bmppath[32] = {0};
	Bmp_display(Pic_BackGround,0,0);
	for( int i = 0 ; i < GameNum ;i++){
			sprintf(bmppath,"resource/digit/digit_%d.bmp",Score[i]); 
			Bmp_display(bmppath,20,120*i);
		}

	Bmp_display(Btn_Return,650,400);
}


/*获取X像素位置*/
int getmatrix_px(int i){
	switch (i)
	{
	case 0:
		return 45;
		break;
	case 1:
		return 150;
		break;
	case 2:
		return 255;
		break;
	case 3:
		return 360;
		break;
	default:
		break;
	}
	
};

/*初始化矩阵*/
void Init_Matrix(){
	int i,j;
	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			matrix[i][j] = 0;
		}
	}
	fillempty();
	fillempty();
}

/*
判断按钮
state 用户所处位置 0主菜单 1游戏界面 2菜单 3计分板
返回值
0非按钮 1上 2左 3下 4右 
5开始游戏 6继续游戏 7菜单 8计分板 9返回主页
*/
int ifbutton(int state,int x,int y){
	switch (state)
	{
		case 0://主菜单
			if(x > 350 && x < 510 && y > 410 && y < 470 && 0 == isnewgame){//继续游戏
				printf("touch KEY_CONTINUEGAME\n");
				return CONTINUEGAME;
			}else if(x > 350 && x < 510 && y > 410 && y < 470 && 1 == isnewgame){//重开游戏
				printf("touch KEY_RESTART\n");
				return STARTGAME;
			}else if(x > 150 && x < 310 && y > 410 && y < 470){//计分板
				printf("touch KEY_RECORD\n");
				return RECORD;
			}else if(x > 550 && x < 710 && y > 410 && y < 470){//退出游戏
				printf("touch KEY_EXIT\n");
				return EXIT;
			}else
				return NOOPTION;
			break;


		case 1://游戏界面 
			if(x>600 && x<680 && y>280 && y<360){//上
				printf("touch KEY_UP\n");
				return UP;
			}else if(x>510 && x<590 && y>370 && y<450 ){//左
				printf("touch KEY_LEFT\n");
				return LEFT;
			}else if(x>600 && x<680 && y>370 && y<450 ){//下
				printf("touch KEY_DOWN\n");
				return DOWN;
			}else if(x>690 && x<770 && y>370 && y<450 ){//右
				printf("touch KEY_RIGHT\n");
				return RIGHT;
			}else if(x>480 && x<660 && y>40 && y<100 ){//使用道具
				printf("touch KEY_USEITEM\n");
				return BOOMER;
			}else if(x>480 && x<660 &&y>120 && y<200 ){//进入菜单
				printf("touch KEY_MENU\n");
				return MENU;
			}else
				return NOOPTION;


		case 2://菜单
			if(x>400 && x<600 && y>70 && y<120){ //继续游戏
				printf("touch KEY_CONTINUEGAME\n");
				return CONTINUEGAME;
			}else if(x>345 && x<545 && y>140 && y<190 ){//重开
				printf("touch KEY_RESTART\n");
				return STARTGAME;
			}else if(x>345 && x<545 && y>210 && y<260 ){//计分板
				printf("touch KEY_RECORD\n");
				return RECORD;
			}else if(x>345 && x<545 && y>280 && y<330 ){//返回主菜单
				printf("touch KEY_RETURNMAIN\n");
				return RETURNMAIN;
			}else
				return NOOPTION;

		case 3://计分板
			if(x>670 && x<780 && y>400 && y<460){ //返回主页
				printf("touch KEY_RETURNMAIN\n");
				return RETURNMAIN;
			}else
				return NOOPTION;

		default:
			break;


	}
}

/*取随机数*/
int getrand(int x){
	srand( (unsigned)time( NULL ) ); 
	return rand()%x+1;//rand()%x表示取x以内的随机数，即取了随机数后再对x取余  x=rand()%(Y-X+1)+X 
}

void fun_up()
{
    for (int i = 0; i < 4; i++){
        for (int k = 0, j = 1; j < 4; j++){
            if (matrix[i][j] > 0) {
                if(matrix[i][k] == matrix[i][j]){
                    Score[GameNum] +=  matrix[i][k++] *= 2;
                    matrix[i][j] = 0; 
                }else if (matrix[i][k] == 0){
                    matrix[i][k] = matrix[i][j];
                    matrix[i][j] = 0;
                }else{
                    matrix[i][++k] = matrix[i][j];
                    if (k != j) matrix[i][j] = 0;
                }
            }
        }
    }
}
 
void fun_down()
{
    for (int i = 0; i < 4; i++){
        for (int k = 3, j = 2; j >= 0; j--){
            if (matrix[i][j] > 0){
                if (matrix[i][k] == matrix[i][j]){
                    Score[GameNum] += matrix[i][k--] *= 2;
                    matrix[i][j] = 0;
                }else if (matrix[i][k] == 0){
                    matrix[i][k] = matrix[i][j];
                    matrix[i][j] = 0;
                }else{
                    matrix[i][--k] = matrix[i][j];
                    if (k != j) matrix[i][j] = 0;
                }
            }
        }
    }
}

void fun_left()
{
    for (int i = 0; i < 4; i++){
        for (int k = 0, j = 1; j < 4; j++){
            if (matrix[j][i] > 0) {
                if (matrix[j][i] == matrix[k][i]){
                    Score[GameNum] += matrix[k++][i] *= 2;
                    matrix[j][i] = 0;
                }else if (matrix[k][i] == 0){
                    matrix[k][i] = matrix[j][i];
                    matrix[j][i] = 0;   
                }else{
                    matrix[++k][i] = matrix[j][i];
                    if (k != j) matrix[j][i] = 0;
                }
            }
        }
    }
}

void fun_right()
{
    for (int i = 0; i < 4; i++){
        for (int k = 3, j = 2; j >= 0; j--){
            if (matrix[j][i] > 0){
                if (matrix[j][i] == matrix[k][i]){
                    Score[GameNum] += matrix[k--][i] *= 2;
                    matrix[j][i] = 0;
                }
                else if (matrix[k][i] == 0){
                    matrix[k][i] = matrix[j][i];
                    matrix[j][i] = 0;
                }
                else{
                    matrix[--k][i] = matrix[j][i];
                    if (k != j) matrix[j][i] = 0;
                }
            }
        }
    }
}

/*使用炸弹道具*/
void fun_useboomer(){
	matrix[getrand(3)][getrand(3)] = 0;
}
/*开始新游戏*/
void fun_startnewgame(){
	Init_LCD();
	Disp_gamepadui();
	Init_Matrix();
	isnewgame = 0;
	GameNum++;
	instart = 0 , ingame = 1 , inrecord = 0 ,inprogram = 1 , inmenu = 0;
}

/*继续游戏*/
void fun_continuegame(){
	Init_LCD();
	Disp_gamepadui();
	instart = 0 , ingame = 1 , inrecord = 0 ,inprogram = 1 , inmenu = 0;
}

/*打开菜单*/
void fun_openmenu(){
	instart = 0 , ingame = 0 , inrecord = 0 ,inprogram = 1 , inmenu = 1;
		Disp_menu();
		while (inmenu)
		{
			get_user_touch();
			userbehavior = ifbutton(2,touch_x,touch_y);
			switch (userbehavior)
			{
			case CONTINUEGAME:
				fun_continuegame();
				break;
			case STARTGAME:
				fun_startnewgame();
				break;
			case RECORD:
				fun_openrecord();
				break;
			case RETURNMAIN:
				fun_openmain();
				break;
			default:
				break;
			}
			touch_x=-1,touch_y=-1,userbehavior=0;
		}
}

/*打开计分板*/
void fun_openrecord(){
	Init_LCD();
	Disp_record();
	instart = 0 , ingame = 0 , inrecord = 1 ,inprogram = 1 , inmenu = 0;
	while(inrecord){
		Disp_record();
		get_user_touch();
		userbehavior = ifbutton(3,touch_x,touch_y);
		switch (userbehavior)
		{
			case RETURNMAIN:
				fun_openmain();
				break;
			default:
				break;
		}
		touch_x=-1,touch_y=-1,userbehavior=0;
	}
}

/*打开开始界面*/
void fun_openmain(){
	instart = 1 , ingame = 0 , inrecord = 0 ,inprogram = 1 , inmenu = 0;
	while(instart){
			Disp_startscreen(!isnewgame);
			get_user_touch();
			userbehavior = ifbutton(0,touch_x,touch_y);
			switch (userbehavior)
			{
				case STARTGAME:
					fun_startnewgame();
					break;
				case CONTINUEGAME:
					fun_continuegame();
					break;
				case RECORD:
					fun_openrecord();
					break;
				case EXIT:
					Init_LCD();
					instart = 0;
					inprogram = 0;
				default:
					break;
			}
			touch_x=-1,touch_y=-1,userbehavior=0;
		}
}

/* 游戏输赢判断
return 1 胜利
reutrn 0 失败
return 3 无事发生
*/
int ifGameOver(){
	if ( Get_Zero_num() ==0 )
	{
		
		int list1 = 0, list2 = 0;//一个接一个对比
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 3; j++){
				if (matrix[i][j] != matrix[i][j + 1])
					list1++;
				if (matrix[i][j] >= 2048)//游戏胜利
					return 1;
			}
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 3; j++)
				if (matrix[j][i] != matrix[j + 1][i])
					list2++;      
		}
		if (list1 == 12 && list2 == 12)
			return 0;//游戏失败
		return 3;
		
	}
	return 3;
}

/*主函数*/
int main()
{
	//1.打开屏幕
	Init_LCD();
	
	while (inprogram)
	{
		//2.进入开始界面
		fun_openmain();
		
		//进入主界面
		while(ingame){
			//主界面
			
			Disp_nummatrix();
			get_user_touch();
			userbehavior = ifbutton(1,touch_x,touch_y);
			int old_matrix[4][4];//用来存旧的数组
			Save_array(old_matrix,4);
			switch (userbehavior)
			{
				case UP:
					fun_up();
					break;
				case LEFT:
					fun_left();
					break;
				case DOWN:
					fun_down();
					break;
				case RIGHT:
					fun_right();
					break;
				case BOOMER:
					fun_useboomer();
					continue;
					break;
				case MENU:
					fun_openmenu();
					continue;
					break;
				case NOOPTION:
					continue;
				default:
					break;
			}
			if(Get_Zero_num()!=0 && ischange(old_matrix))
			{
				fillempty();
			}
			if(ifGameOver() == 1){
				Bmp_display(Pic_Victory,0,0);
				printf("victory!");
				sleep(3);
				isnewgame=1;
				fun_openmain();	
			}else if(ifGameOver() == 0){
				Bmp_display(Pic_Lose,0,0);
				printf("YOU LOSE!!");
				sleep(3);
				isnewgame=1;
				fun_openmain();
			}else if(ifGameOver() == 3){
				
			}
			touch_x=-1,touch_y=-1,userbehavior=0;
		}
		
	//3.关闭屏幕
	Unit_LCD();	
	return 0;
	}
}
