#include<stdio.h>
#include<unistd.h>

char data [1024*1024*256];

int main()
{
	int i;

	for ( i = 0;i< 1024*1024*256;i+=1024)
	{
		data[i] = 'c';
	}

	printf("The current break point is %p\n",sbrk(0));
	sbrk(-1024*1024*128);
	sleep(15);
	printf("chaging brk\n");
	printf("The current break point is %p\n",sbrk(0));
	sleep(1000);

}
