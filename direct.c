#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



int main()
{
	int fd;
	char buff[1024*1024];
	void *b;
	int i;
	for(i = 0;i<1024*1024;i++)
	{
		buff[i] = 'c';
	}
	b = buff;
	fd = open("testfile",O_CREAT|O_DIRECT|O_WRONLY|O_ASYNC);
	write(fd,b,1024);
	close(fd);
}
