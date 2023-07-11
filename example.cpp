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
  
  printf( "%d\n", poa.Add(s2, strlen(s2))) ;
  printf( "%d\n", poa.Add(s2, strlen(s2))) ;
  printf( "%d\n", poa.Add(s3, strlen(s3))) ;
  printf( "%d\n", poa.Add(s4, strlen(s4))) ;
  printf( "%d\n", poa.Add(s4, strlen(s4))) ;
  printf( "%d\n", poa.Add(s4, strlen(s4))) ;
  //poa.VisualizePOA() ;
  
  char *consensus = poa.Consensus() ;
  printf("consensus = %s\n", consensus) ;
  
  free(consensus) ;
  return 0 ;
}
