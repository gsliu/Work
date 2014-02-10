#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FILE_SIZE 1024*1024
int main(int argc, char *argv[])
{
    int i;
    int fd = open("sparse1.file", O_RDWR|O_CREAT);
   for ( i = 0;i<FILE_SIZE/2;i++)
  {
   	 lseek(fd, i, SEEK_CUR);
       	write(fd, "f", 1);
	}

   	lseek(fd, FILE_SIZE, SEEK_CUR);
       	write(fd, "f", 1);
    return 0;
}
