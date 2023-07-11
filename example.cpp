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
  std::vector< std::pair<int, int> > path ;
  poa.Align(s2, strlen(s2), path) ;
  return 0 ;
}
