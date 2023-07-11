#ifndef _MOURISL_POA
#define _MOURISL_POA

// Stand-alone class for partial order alignment

#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct _poaNode
{
  char c ;
  std::vector< std::pair<int, int> > next ;
  std::vector<int> prev ;
} ;

struct _poaAlignScore
{
  int score ;
  std::pair<int, int> prev ; //first:i, second:j 
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
      inCnt[i] = _nodes[i].prev.size() ; 

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
        --inCnt[ _nodes[i].next[j].first ] ; 
        if (inCnt[ _nodes[i].next[j].first ] == 0)
        {
          queue[tail] = _nodes[i].next[j].first ;
          ++tail ;
        }
      }
    }
    free(inCnt) ;
    free(queue) ;
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
  // path: the path of the alignment, starting from (0,0):first:seq position, second: 
  // return: match score
  int Align(char *seq, int len, std::vector< std::pair<int, int> > &path)
  {
    int i, j, k ;
    
    int size = _nodes.size() ;

    // Topological sort.
    std::vector<int> sortNodes ;
    TopologicalSort(sortNodes) ;
    std::vector<int> sinks ;
    for (i = 0 ; i < size ; ++i)
      if (_nodes[i].next.size() == 0)
        sinks.push_back(i) ;

    // Follow the topological order to fill the scoreMatrix matrix.
    struct _poaAlignScore **scoreMatrix = NULL ;
    scoreMatrix = (struct _poaAlignScore **)malloc(sizeof(struct _poaAlignScore *) * (len+1) ) ; // row is the input sequence
    for (i = 0 ; i <= len ; ++i)
      scoreMatrix[i] = (struct _poaAlignScore *)calloc(size, sizeof(struct _poaAlignScore)) ; // column is the POA     
    scoreMatrix[0][0].score = 0 ;
    scoreMatrix[0][0].prev.first = scoreMatrix[0][0].prev.second = -1 ;

    for (i = 0 ; i <= len ; ++i)
    {
      for (j = 1 ; j < size ; ++j)
      {
        int prevSize = _nodes[j].prev.size() ;
        int match = _matScore ;
        if (i > 0 && seq[i - 1] != _nodes[j].c)
          match = _misScore ;
        for (k = 0 ; k < prevSize ; ++k)
        {
          // Deletion to the graph
          int prevj = _nodes[j].prev[k] ;
          int score = scoreMatrix[i][prevj].score + _delScore ; 
          std::pair<int, int> prev ;
          prev.first = i ;
          prev.second = prevj ;
          if (i > 0)
          {
            // match/mismatch
            if (scoreMatrix[i - 1][prevj].score + match > score)
            {
              score = scoreMatrix[i][prevj].score + _insScore ;
              prev.first = i - 1 ;
              prev.second = prevj ;
            }

            // Insertion to the graph
            if (i > 0 && scoreMatrix[i - 1][j].score + _insScore > score)
            {
              score = scoreMatrix[i - 1][j].score + _insScore ;
              prev.first = i - 1 ;
              prev.second = j ;
            }
          }
          
          scoreMatrix[i][j].score = score ;
          scoreMatrix[i][j].prev = prev ;
        }
      }
    }

    // Find the good sink.
    int sinkSize = sinks.size() ;
    int maxScore = 0 ;
    int maxtag = -1 ;
    for (i = 0 ; i < sinkSize ; ++i)
    {
      if (maxtag == -1 || scoreMatrix[len][ sinks[i] ].score > maxScore)
      {
        maxtag = sinks[i] ;
        maxScore = scoreMatrix[len][ sinks[i] ].score ;
      }
    }

    // Traceback
    std::pair<int, int> pos ;
    pos.first = len ;
    pos.second = maxtag ;
    while (pos.first > 0 || pos.second > 0)
    {
      path.push_back(pos) ;
      pos = scoreMatrix[ pos.first ][ pos.second ].prev ; 
    }
    path.push_back(pos) ;
   
    // Reverse the path
    int pathSize = path.size() ;
    for (i = 0, j = pathSize - 1 ; i < j ; ++i, --j )
    {
      std::pair<int, int> tmpp = path[i] ;
      path[i] = path[j] ;
      path[j] = tmpp ;
    }

    for (i = 0 ; i <= len ; ++i)
      free(scoreMatrix[i]) ;
    free(scoreMatrix) ;

    return maxScore ;
  }

  // Add a sequence to the graph based on the alignment
  void Add(char *seq, int len, int *align, int *path)
  {
    
  }

  void Consensus(char *seq, int &len)
  {

  }

  void VisualizePOA()
  {

  }

  void VisualizeAlignment(char *seq, int len, std::vector< std::pair<int, int> > path)
  {
  }
} ;

#endif
