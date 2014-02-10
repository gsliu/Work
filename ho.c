#include<asm/unistd.h>

static int errno;

#define _syscall1(type,name,type1, arg1)

#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3)

_syscall1(int,exit,int,status);

_syscall3(int,write,int,fd,const void*,buff,unsigned loong,count);

void hello(){
	write(1,"Hello world!\n",13);
	exit(0);
}

