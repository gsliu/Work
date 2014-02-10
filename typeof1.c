#include <stdio.h>

#define array(type, size) typeof(type [size])

int func(int num)
{
    return num + 5;
}

int main(void)
{
    typeof (func) *pfunc = func; //等价于int (*pfunc)(int) = func;
    printf("pfunc(10) = %d\n", (*pfunc)(10));

    array(char, ) charray = "hello world!"; //等价于char charray[] = "hello world!";
    typeof (char *) charptr = charray; //等价于char *charptr = charray;

    printf("%s\n", charptr);

    return 0;
}

