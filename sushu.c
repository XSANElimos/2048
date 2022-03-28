// 任意数区间的素数和
#include<stdio.h>
#define MIN 1 //求素数起始
#define MAX 100  //求素数终止点

int isKey(int key){ // 判断是否是素数 
	for( int i = 2 ; i < key -1; i++){
		//printf("%d  ",key % i);
		if(key % i == 0){
			return 1;
		}
		else continue;
	}
	return 0;
}

int main(){
	
	int sum = 0 , count = 0;

	for(int i = MIN ; i <= MAX ; i++ ){

		if ( i == 1 ) continue;

		if( isKey( i ) == 0 ){
			count++;

			printf(" %d是素数 ",i);

			if( count == 10 ) {
				printf("\n");
				count = 0;
			}
			
			sum = sum + i;
		}
	}
	printf("\n%d 到 %d 的素数和是 %d ",MIN,MAX,sum);
}
