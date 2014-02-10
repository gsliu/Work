#include<stdio.h>

struct list_head{
	struct list_head * pre, *next;
};
struct my_struct{
    const char *name;
    int index;
    struct list_head list;
};

int main()
{
	struct my_struct * m;
	struct my_struct * n;
	struct list_head * l;
	m = (struct my_struct *) malloc(sizeof(struct my_struct));
	m->name = "Hello";
	l=&m->list;

	n=(struct my_struct*)((int)l+sizeof(char *)+sizeof(int));

	printf("size of my_struct is %d\n", sizeof(struct my_struct));

	printf("%d\n", (int)m-(int)l);

	printf("%s\n", n->name);
       


	return 1;
}
