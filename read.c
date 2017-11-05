#include <stdio.h>
#include <stdlib.h>
int main(){
	int fd=open("test/mount/e1",0);
	if(fd<0)printf("Err , not file...");
	char *buff=(char *)malloc(sizeof(char *)*12);
	read(fd,buff,12);
	printf("%s",buff);
	return 0;
}
