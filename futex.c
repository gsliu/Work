#include<stdio.h>
#include <linux/futex.h>
#include <sys/time.h>
int main()
{
	syscall(SYS_futex , &futex_var, FUTEX_WAKE, 1, NULL, NULL, NULL);
	sleep(100);
}
