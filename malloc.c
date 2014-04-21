#include<stdio.h>

int main()
{
	int i;
	char * array;
	array = malloc(1024*1024*256);
	for(i = 0; i < 1024*1024*256;i+=4096)
	{
		array[i] = 'c';
		sleep(1);
	}
	sleep(100000);
}
