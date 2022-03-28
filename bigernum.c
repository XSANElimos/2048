#include <stdio.h>

int bigernum(int * a){
    int length = 10;
    printf("length is :%d\n",length);
    if(length <= 2){ printf("invaild input!"); return 0; }
    int maxnum = a[0] , bigernum = a[0]<a[1]?a[0]:a[1]; 
    for( int i = 1 ; i < length ; i++ ){
        if ( a[i] > maxnum ){
            bigernum = maxnum;
            maxnum = a[i];
        }
        if ( a[i] > bigernum && a[i] < maxnum ){
            bigernum = a[i];
        }
    }
    return bigernum;
}

int main(){
    int  a[10] = {10,1,2,3,4,5,9,6,7,8};
    printf("\nbiger num is %d",bigernum(a));
    return 0;
}