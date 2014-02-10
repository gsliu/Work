#include<stdio.h>

int funct(int p)
{
	int i;
	char array[128] ;
	for(i = 0;i< 128;i++)
		array[i] = 'c';
	if(p == 0)
	{
		sleep(100000);
		return 0;
	}else{
		return funct(p-1);
	}
}

int main()
{
	funct(20);
	return 0;
}

