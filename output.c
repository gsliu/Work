/* cumulative CPU time, [dd-]hh:mm:ss format (not same as "etime") */
#include<stdio.h>

 int pr_time(char * outbuf, long time){
  unsigned long t, qq;
  unsigned dd,hh,mm,ss;
  int c;
  t = time / 100;
  ss = t%60;
  t /= 60;
  mm = t%60;
  t /= 60;
  hh = t%24;
  t /= 24;
  dd = t;
  //c  =( dd ? snprintf(outbuf, COLWID, "%u-", dd) : 0              );
   sprintf(outbuf,  "%u- %02u:%02u:%02u\n", dd, hh, mm, ss)    ;
  return c;
}

int main()
{
	char * str;

	long time =  2+ 1844674407370 ;

	pr_time(str, time);
	printf("%s", str);
}
