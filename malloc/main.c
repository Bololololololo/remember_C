#include "bolloc.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
   int* p = (int*) bolloc(10 * sizeof(int));
   int* q = (int*) bolloc(10 * sizeof(int));

   if( p == NULL ) {
	   printf("FAIL.\n");
   } else {
	   printf("SUCCESS.\n");
   }

   for( int i = 0; i < 20; i++ ) {
      p[i] = i;
      printf("p[%d] = %d\n", i, p[i]);
   }
   
   for( int i = 0; i < 10; i++ ) {
      printf("q[%d] = %d\n", i, q[i]);
   }
   
   return 0;
}
