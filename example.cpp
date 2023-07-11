#include <stdio.h>

#include "POA.hpp"

int main()
{
  POA poa ;
  char s1[] = "ACGTACGT" ;
  char s2[] = "ACGGACGT" ;
  char s3[] = "ACGTTACGT" ;
  char s4[] = "ACGACGT" ;

  poa.Init(s1, strlen(s1)) ;
  printf( "%d\n", poa.Add(s2, strlen(s2))) ;
  printf( "%d\n", poa.Add(s2, strlen(s2))) ;
  char *consensus = poa.Consensus() ;
  printf("%s\n", consensus) ;
  free(consensus) ;
  return 0 ;
}
