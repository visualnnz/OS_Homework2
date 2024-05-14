#include <stdio.h>
#include <stdint.h>
#include "smalloc.h"

int main ()
{
	void *p1, *p2, *p3, *p4, *p5, *p6, *p7 ;
	int i = 1;

	smdump() ;

	p1 = smalloc(2000) ; // 1
	printf("%d. smalloc(2000):%p\n", i, p1) ;
	i++;
	smdump() ;

	p2 = smalloc(1000) ; // 2
	printf("%d. smalloc(1000):%p\n", i, p2) ;
	i++;
	smdump() ;

	p3 = smalloc(3000) ; // 3
	printf("%d. smalloc(3000):%p\n", i, p3) ;
	i++;
	smdump() ;

	p4 = smalloc(2000) ; // 4
	printf("%d. smalloc(2000):%p\n", i, p4) ;
	i++;
	smdump() ;

	sfree(p1) ; // 5
	printf("%d. sfree(%p)\n", i, p1) ;
	i++;
	smdump() ;

	sfree(p2) ; // 6
	printf("%d. sfree(%p)\n", i, p2) ;
	i++;
	smdump() ;

	smcoalesce(); // 7
	printf("%d. smcoalesce()\n", i) ;
	i++;
	smdump();

	p5 = smalloc_mode(1500, worstfit) ; // 8
	printf("%d. smalloc_mode(1500, worstfit):%p\n", i, p5) ;
	i++;
	smdump() ;

	p6 = smalloc_mode(2000, worstfit) ; // 9
	printf("%d. smalloc_mode(2000, worstfit):%p\n", i, p6) ;
	i++;
	smdump() ;

	p7 = smalloc_mode(1500, worstfit) ; // 10
	printf("%d. smalloc_mode(1500, worstfit):%p\n", i, p7);
	i++;
	smdump() ;
	
}
