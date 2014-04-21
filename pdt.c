#include<stdio.h>

typedef struct { unsigned long pgd; } pgd_t;

int main(){
  pgd_t * p;
  printf ("size of pgd_t = %d\n", sizeof(pgd_t));
  p = malloc(sizeof(pgd_t));
  *(unsigned long *) p=100;
  p->pgd=200;
  printf("pgd = %lu\n", *(unsigned long*)p);
  printf("pgd->pgd = %lu\n", p->pgd);
}

