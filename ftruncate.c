#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
int main()
{
	int fd;
	int ret;
	off_t l;
	fd = fopen("/root/work/a","rw");
	l = 16;
	printf("off_t is %d", sizeof(off_t));

	ret = truncate( "/root/work/a",  l);

	close(fd);
}
