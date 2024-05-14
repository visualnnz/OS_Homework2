#include <stdio.h>
#include <stdint.h>
#include "smalloc.h"

int main ()
{
	void *p1, *p2, *p3, *p4, *p5 ;
	int i = 1;

	smdump() ;

	p1 = smalloc(1000) ; // 1
	printf("%d. smalloc(1000):%p\n", i, p1) ;
	i++;
	smdump() ;

	p2 = smalloc(1000) ; // 2
	printf("%d. smalloc(1000):%p\n", i, p2) ; 
	i++;
	smdump() ;

	p3 = smalloc(1000) ; // 3
	printf("%d. smalloc(1000):%p\n", i, p3) ;
	i++;
	smdump() ;

	p4 = smalloc(3000) ; // 4
	printf("%d. smalloc(3000):%p\n", i, p4) ;
	i++;
	smdump() ;

	srealloc(p2, 500) ; // 5
	printf("%d. srealloc(p2, 500):%p\n", i, p2) ;
	i++;
	smdump() ;

	srealloc(p4, 1500) ; // 6
	printf("%d. srealloc(p4, 1500):%p\n", i, p4) ;
	i++;
	smdump() ;

	sfree(p1) ; // 7
	printf("%d. sfree(%p)\n", i, p1) ;
	i++;
	smdump() ;

	sfree(p3) ; // 8
	printf("%d. sfree(%p)\n", i, p3) ;
	i++;
	smdump() ;

	smcoalesce(); // 9
	printf("%d. smcoalesce()\n", i) ;
	i++;
	smdump();

	p5 = smalloc_mode(2500, firstfit); // 10
	printf("%d. smalloc_mode(p5, firstfit):%p\n", i, p5) ;
	i++;
	smdump() ;
	
}
