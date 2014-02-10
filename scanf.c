#include <stdio.h>

int main ()
{
  char str [10];
  int i;
  scanf ("%s",str);  
  printf ("%s\n", str);
  for( i=0;i<100;i++)
  {
	  printf("address is %lx, value is %c\n",&str[i],str[i]);
  }

  sleep(100000);
  
  return 0;
}
