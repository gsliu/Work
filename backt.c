#include<stdio.h>

typedef struct layout {
	struct layout *ebp;
	void *ret;
} layout;

void print_backtrace(){
	layout *ebp = __builtin_frame_address(0);
	while(ebp){
	  :		printf("0x%08x\n",ebp->ret);
		ebp=ebp->ebp;
	}
}

void foo(){
	print_backtrace();
}

int main(){
	foo();
	return 0;
}
