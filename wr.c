#include <stdio.h>
#include <stdlib.h>


int main(){
	int size;
	int i;
	printf("give size -> ");
	scanf("%d",&size);
	char str[size];
	scanf("%s",str);
	int fd=open("test/mount/e1","wr");
	if(fd<0)printf("err in open\n");
	if(write(fd,str,size)!=size){
		printf("write () fail\n");
//		return 0;
	}
	printf("Wait for read...\n");
//	fflush(stdin);
	scanf("%d",i);
	if(read(fd,str,size)){
		printf("error...\n");
	}
	printf("Read: %s",str);
	return 0;
}
