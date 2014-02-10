#include<stdio.h>

int main()
{
	char a[1000];
	char b[1000];
	int i;
	for(i = 0;i < 100; i++)
	{
		a[i] = 'c';
		b[i] = 'd';
	}

	printf("%c\n", b[10000]);
}
