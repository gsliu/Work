#include<execinfo.h>

void foo(){
	void * trace[128];
	int n = backtrace(trace,sizeof(trace)/sizeof(trace[0]));
	backtrace_symbols_fd(trace,n,1);
}
void f2(){
	foo();
}

void f3(){
	f2();
}

int main(){
	f3();
	return 0;
}
