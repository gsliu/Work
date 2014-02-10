#include<stdio.h>

int main()
{
	char * p;
	int i = 0;
	for(;i < 1024*1024; i++)
	{
		p = malloc( sizeof(char));
		//free(p);
	}
	return 0;
}
