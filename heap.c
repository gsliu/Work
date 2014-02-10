#include<sys/mman.h>
#include<unistd.h>
double func(void){
	return 3.14;
}
double funcb(void){
	return 0;
}

void allow_execution(const void *addr){
///	long pagesize = (int)(sysconf(_SC_PAGESIZE));
	long pagesize = 4096;
	char *p = (char*)(((long)addr & ~(pagesize) - 1L));
	mprotect(p,pagesize*100L, PROT_READ|PROT_WRITE|PROT_EXEC);
}

void not_allow_execution(const void *addr){
///	long pagesize = (int)(sysconf(_SC_PAGESIZE));
	long pagesize = 4096;
	char *p = (char*)(((long)addr & ~(pagesize) - 1L));
	mprotect(p,pagesize*100L, PROT_READ|PROT_EXEC);
}





int main(int argc, char**argv){


	allow_execution((void *)0x400000);
	//sleep(1000);
	
	memcpy(&funcb,&func,100);
	printf("PI = %g\n",funcb());
	not_allow_execution((void *)0x400000);
	pause();
}
