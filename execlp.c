#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main()
{
	printf("Running ps with execlp\n");
	execlp("ps","ps","ax",0);
	printf("done\n");
	exit(0);
}

