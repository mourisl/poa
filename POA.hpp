#ifndef _MOURISL_POA
#define _MOURISL_POA

// Stand-alone class for partial order alignment

#include <vector>
#include <stdio.h>

struct _poaNode
{
  char c ;
  std::vector< std::pair<int, int> > next ;
} ;

class POA
{
private:
  std::vector<struct _poaNode> _nodes ;
  int _matScore ; 
  int _misScore ;
  int _insScore ;
  int _delScore ;

  void AddEdge(int from, int to, int weight)
  {
    int i ;
    int size = _nodes[from].next.size() ;
    for (i = 0 ; i < size ; ++i)
      if (_nodes[from].next[i].first == to)
      {
        _nodes[from].next[i].second += weight;
        return ;
      }
    std::pair<int, int> np ;
    np.first = from ;
    np.second = weight ;
    _nodes[from].next.push_back(np) ;
  }

public:
  POA() 
  {
    _matScore = 0 ;
    _misScore = -2 ;
    _insScore = -4 ;
    _delScore = -4 ;
  }

  ~POA() {}

  void Init(char *seq, size_t len)
  {
    size_t i ;
    for (i = 0 ; i < len ; ++i)
    {
      struct _poaNode nd ;
      nd.c = seq[i] ;
      _nodes.push_back(nd) ;
      if (i > 0)
        AddEdge(i - 1, i, 1) ;
    }
  }

  // Align a sequence to the graph
  // return: match score
  int Align(char *seq, int len, int *align, int *path)
  {
  }

  // Add a sequence to the graph based on the alignment
  void Add(char *seq, int len, int *align, int *path)
  {
  }

  void Consensus(char *seq, int &len)
  {
  }

  void Visualize()
  {
  }
} ;

#endif
