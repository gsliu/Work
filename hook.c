#include<stdio.h>

long time(void *unused)
{
	printf("hooked\n");
	return 123456;
}
