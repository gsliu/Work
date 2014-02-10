#include<stdio.h>
int main()
{
	int i=1;
	int move = 1;
	for ( move = 1; move < 16; move ++)
	{
		printf("move is %d, i is %d \n", move, i<<move);
	}
}
