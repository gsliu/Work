/**
 *  * map.c
 *   */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define NSIZE   (5*(1<<20))
#define SLEEPT  100

long  gx[NSIZE];

int main (int argc, char *argv[])
{
	  char  c[NSIZE];
	  int  *px = malloc (NSIZE*sizeof(int));

	  for (int i=0; i<NSIZE; i++) {
	      gx[i] = (long)i;
	      px[i] = i;
	      c[i]  = 'c';
     	   }
       	printf ("address of gx[0] = %012p\n", &gx[0]);
       	printf ("address of gx[1] = %012p\n", &gx[1]);
 	 printf ("address of px[0] = %012p\n", &px[0]);
 	 printf ("address of px[1] = %012p\n", &px[1]);
    printf ("address of  c[0] = %012p\n", &c[0]);
    printf ("address of  c[1] = %012p\n", &c[1]);

      printf ("memory map file: /proc/%d/maps\n", getpid());
        printf ("sleeping %d...", SLEEPT);
	  fflush (NULL);
    sleep (SLEEPT);

	      free (px);

      printf ("\ndone\n");
  exit (EXIT_SUCCESS);
}
