#include <stdio.h>
#include <stdint.h>
#include "smalloc.h"

int main ()
{
	void *p1, *p2, *p3, *p4, *p5, *p6 ;
	int i = 1;

	smdump() ;

	p1 = smalloc(3000) ; // 1
	printf("%d. smalloc(3000):%p\n", i, p1) ;
	i++;
	smdump() ;

	p2 = smalloc(3000) ; // 2
	printf("%d. smalloc(3000):%p\n", i, p2) ;
	i++;
	smdump() ;

	p3 = smalloc(2000) ; // 3
	printf("%d. smalloc(2000):%p\n", i, p3) ;
	i++;
	smdump() ;

	p4 = smalloc(2000) ; // 4
	printf("%d. smalloc(2000):%p\n", i, p4) ;
	i++;
	smdump() ;

	p5 = smalloc(3500) ; // 5
	printf("%d. smalloc(3500):%p\n", i, p5) ;
	i++;
	smdump() ;

	sfree(p3) ; // 6
	printf("%d. sfree(%p)\n", i, p3) ;
	i++;
	smdump() ;

	sfree(p4) ; // 7
	printf("%d. sfree(%p)\n", i, p4) ;
	i++;
	smdump() ;

	smcoalesce(); // 8
	printf("%d. smcoalesce()\n", i) ;
	i++;
	smdump();

	srealloc(p1, 4000) ; // 9
	printf("%d. srealloc(p1, 4000):%p\n", i, p1) ;
	i++;
	smdump() ;

	srealloc(p2, 2500) ; // 10
	printf("%d. srealloc(p2, 2500):%p\n", i, p2) ;
	i++;
	smdump() ;

	p6 = smalloc_mode(500, bestfit) ; // 11
	printf("%d. smalloc_mode(p6, bestfit):%p\n", i, p6) ;
	i++;
	smdump() ;
	
}
