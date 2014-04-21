#include <stdio.h>
#include<sys/types.h>
int main()
{
  if(!fork()){
	printf("child pid=%d\n", getpid());
	exit(0);
  }
/*wait();*/
/*waitpid(-1,NULL,0);*/
  wait();
  sleep(60);
  printf("parent pid=%d \n", getpid());
  exit(0);
}
