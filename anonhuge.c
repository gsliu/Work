#include<stdio.h>

int main()
{
	int i;
	char * array;
	array = malloc(1024*1024*1024);

	for ( i = 0; i < 1024*1024*1024; i+= 1)
		array[i] = 'c';
	sleep(100011);
}

