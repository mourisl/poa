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

  std::vector< std::pair<int, int> > align ;
  poa.Align(s3, strlen(s3), align) ;
  poa.VisualizeAlignment(s3, strlen(s3), align) ;
  
  printf( "%d\n", poa.Add(s3, strlen(s3))) ;
  poa.VisualizePOA() ;
  printf( "%d\n", poa.Add(s3, strlen(s3))) ;
  char *consensus = poa.Consensus() ;
  printf("%s\n", consensus) ;
  
  free(consensus) ;
  return 0 ;
}
