#include <stdio.h>
#include <stdint.h>
#include "smalloc.h"

int main ()
{
	void *p1, *p2, *p3, *p4;
	void *t1, *t2, *t3, *t4, *t5; // for test

	int i = 1;

	smdump();

	p1 = smalloc(2000); // 1
	printf("%d. smalloc(2000):%p\n", i, p1); 
	smdump();
	i++;

	p2 = smalloc(2500); // 2
	printf("%d. smalloc(2500):%p\n", i, p2); 
	smdump();
	i++;

	sfree(p1); 
	printf("%d. sfree(%p)\n", i, p1); 
	smdump();
	i++;

	p3 = smalloc(1000); // 3
	printf("%d. smalloc(1000):%p\n", i, p3); 
	smdump();
	i++;

	p4 = smalloc(1000); // 4
	printf("%d. smalloc(1000):%p\n", i, p4); 
	smdump();
	i++;

	// /* **for test** */
	// t1 = smalloc(1600); // 5
	// printf("%d. smalloc(1600):%p\n", i, t1); 
	// smdump();
	// i++;

	// t2 = smalloc(1700); // 6
	// printf("%d. smalloc(1700):%p\n", i, t2); 
	// smdump();
	// i++;

	// t3 = smalloc(1800); // 7
	// printf("%d. smalloc(1800):%p\n", i, t3); 
	// smdump();
	// i++;

	// t5 = smalloc(1300); // 7
	// printf("%d. smalloc(1300):%p\n", i, t5); 
	// smdump();
	// i++;

	// t4 = smalloc(1900); // 8
	// printf("%d. smalloc(1900):%p\n", i, t4); 
	// smdump();
	// i++;
	// /* **for test** */
}
