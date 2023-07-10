#ifndef _MOURISL_POA
#define _MOURISL_POA

// Stand-alone class for partial order alignment

#include <vector>
#include <stdio.h>

struct _poaNode
{
  char c ;
  std::vector< std::pair<int, int> > next ;
  std::vector< std::pair<int, int> > prev ;
} ;

struct _poaAlignScore
{
  int score ;
  int prev[2] ; //i, j
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
    {
      if (_nodes[from].next[i].first == to)
      {
        _nodes[from].next[i].second += weight;
        return ;
      }
    }
  
    std::pair<int, int> np ;
    np.first = to ;
    np.second = weight ;
    _nodes[from].next.push_back(np) ;
    
    _nodes[to].prev.push_back(from) ;
  }

  void TopologicalSort(std::vector<int> &ret)
  {
    int i, j ;
    int size = _nodes.size() ; 
    
    ret.clear() ;
    int *inCnt = (int *)calloc(size, sizeof(int)) ;
    for (i = 0 ; i < size ; ++i)
      inCnt[size] = _nodes.prev.size() ; 

    // The graph should be a DAG.
    int *queue = (int *)malloc(size * sizeof(int)) ;
    int head = 0 ;
    int tail = 1 ;
    queue[head] = 0 ; // source
    while (head < tail)
    {
      i = queue[head] ;
      ret.push_back( queue[head] ) ; 
      ++head ;

      int nextSize = _nodes[i].next.size() ;
      for (j = 0 ; j < nextSize ; ++j)
      {
        --inCnt[ _nodes[i].next[j] ] ; 
        if (inCnt[ _nodes[i].next[j] ] == 0)
        {
          queue[tail] = _nodes[i].next[j] ;
          ++tail ;
        }
      }
    }
  }

  // Max score
  int Max(int a, int b) const
  {
    return a >= b ? a : b ;
  }
public:
  POA() 
  {
    _matScore = 0 ;
    _misScore = -2 ;
    _insScore = -4 ;
    _delScore = -4 ;
    
    struct _poaNode nd ;
    nd.c = '\0' ; // source node
    _nodes.push_back(nd) ;
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
        AddEdge(i, i + 1, 1) ;
    }
  }

  // Align a sequence to the graph
  // return: match score
  int Align(char *seq, int len, int *align, int *path)
  {
    int i, j, k ;
    
    int size = _nodes.size() ;

    // Topological sort.
    std::vector<int> sortNodes ;
    TopologicalSort(sortNodes) ;
    
    // Follow the topological order to fill the scoreMatrix matrix.
    struct _poaAlignScore **scoreMatrix = NULL ;
    scoreMatrix = (struct _poaAlignScore **)malloc(sizeof(struct _poaAlignScore *) * len) ; // row is the input sequence
    for (i = 0 ; i < len ; ++i)
      scoreMatrix = (int *)calloc(size, sizeof(struct _poaAligneScore)) ; // column is the POA     
    scoreMatrix[0][0].score 
    for (i = 1 ; i < len ; ++i)
    {
      for (j = 1 ; j < size ; ++j)
      {
        int prevSize = _nodes[j].prev.size() ;
        int match = _matScore ;
        if (seq[i] != _nodes[j].c)
          match = _misScore ;
        for (k = 0 ; k < prevSize ; ++k)
        {
          int prev = _nodes[j].prev[k] ;
          int score = match + scoreMatrix[i - 1][k];
          int op = 0 ; // match
          
          // insertion to the graph
          if (scoreMatrix[i])

          // deletion to the graph
        }
      }
    }
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
