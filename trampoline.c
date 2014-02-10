void other (void (*funcp)()){
	funcp();
}

void outer(void){
	int a=10;
	void inner(void){
		sleep(100000);
		printf("outer's a is %d\n",a);
	}
	other(inner);
}

int main()
{
	outer();
}
