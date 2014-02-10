#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>


#include <fcntl.h>

#define SIZE 1024*1024*128
int main()
{
	char *p;
	char *q;
	int i;
	p = mmap (NULL, /* do not care where */
	SIZE, /* 512 KB */
	PROT_READ | PROT_WRITE, /* read/write */
	MAP_ANONYMOUS | MAP_PRIVATE, /*
	anonymous, private */
	-1, /* fd (ignored) */
	0); /* offset (ignored) */
	for (i = 0;i< SIZE;i++)
	{
		p[i] = 'c';
	}

	printf("p finished\n");
	sleep(10);

	q = mmap (NULL, /* do not care where */
	SIZE, /* 512 KB */
	PROT_READ | PROT_WRITE, /* read/write */
	MAP_ANONYMOUS | MAP_PRIVATE, /*
	anonymous, private */
	-1, /* fd (ignored) */
	0); /* offset (ignored) */
	for (i = 0;i< SIZE;i++)
	{
		q[i] = 'c';
	}
	printf("q finished\n");
	sleep(10000);
}



