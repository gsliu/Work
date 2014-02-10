#include<stdio.h>
#include<signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


char buffer[32];
int fd;

void writetofile(int fd)
{

	printf("write\n");
	close(fd);
	exit(0);
}


int main()
{
	char c = 'c';
	int i;
	int ret;

	//signal(SIGINT, writetofile);

	fd = open("testfile",O_CREAT);
/*
	for( i = 0; i <1024*1024; i++)
	{
		buffer[i] = 'c';
	}
	*/

	ret = read(fd, buffer, 32);

	printf("%d\n", ret);

	for( i = 0; i <ret; i++)
	{
		printf("%c",buffer[i]);
	}

	sleep(1000000);

	 while (1)
 	        pause();


}

	

		
