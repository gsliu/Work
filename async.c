#define _GNU_SOURCE
#include<stdio.h>
#include<sys/fcntl.h>
#include<signal.h>

void sig_handler(int signum)
{

  printf("Signal received\n");
}

int main()
{
  int fd, rv, flags;
  char buff[100];
  signal(SIGIO,sig_handler);
  fd = open("testfile",O_NONBLOCK|O_ASYNC|O_RDWR);
  if(fd < 0)
   {
    printf("open failed\n");
    perror("open");
  }

  sleep(10000);
  /*
  rv = fcntl(fd, F_SETOWN, getpid());
  if(rv < 0 )
   {
    perror("F_SETOWN");
  }
  rv = fcntl(fd, F_GETOWN );
  if(rv < 0 )
   {
    perror("F_SETOWN");
  }

  printf("getown retuened %d\n",rv);

  flags = fcntl(fd, F_GETFL) ;
  rv = fcntl(fd,F_SETSIG,SIGIO);
  rv = fcntl(fd, F_SETFL, flags | O_ASYNC|O_RDWR | O_NONBLOCK);

  if(rv < 0 )
   {
    perror("F_SETFL");
  }
   //pause();
 while(1)
 {
  sleep(3);
  read(fd,buff,4);
  printf("read \n");
 }

 */

  return(0);
}
