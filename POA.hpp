#ifndef _MOURISL_POA
#define _MOURISL_POA

// Stand-alone class for partial order alignment

#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

struct _poaNode
{
  char c ;
  std::vector< std::pair<int, int> > next ; // first: next node id, second: weight
  std::vector<int> prev ;
} ;

struct _poaAlignScore
{
  int score ;
  std::pair<int, int> prev ; //first:i (sequence idx), second:j (graph node id) 
} ;

class POA
{
private:
  std::vector<struct _poaNode> _nodes ;
  int _matScore ; 
  int _misScore ;
  int _insScore ;
  int _delScore ;
  
  const int _sourceId ;
  const int _sinkId ;
  const int _effectiveIdStart ;
  
  int AddNode(char c)
  {
    struct _poaNode n ;
    n.c = c ;
    _nodes.push_back(n) ; 
    return _nodes.size() - 1 ;
  }

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

  // sign of a-b
  int CompareNodeScoreForConsensus(const std::pair<int, int> a, const std::pair<int, int> b)
  {
    return a.first - a.second - b.first + b.second ;
  }
public:
  POA() : _sourceId(0), _sinkId(1), _effectiveIdStart(2) 
  {
    _matScore = 0 ;
    _misScore = -2 ;
    _insScore = -4 ;
    _delScore = -4 ;
    
    AddNode('\0') ; // source
    AddNode('\0') ; // sink
  }

  ~POA() {}

  void Init(char *seq, size_t len)
  {
    size_t i ;
    int nid ;
    for (i = 0 ; i < len ; ++i)
    {
      nid = AddNode(seq[i]) ;
      if (i == 0)
        AddEdge(_sourceId, nid, 1) ;
      else
        AddEdge(nid - 1, nid, 1) ;
    }
    AddEdge(nid, _sinkId, 1) ;
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
    
    // Follow the topological order to fill the scoreMatrix matrix.
    struct _poaAlignScore **scoreMatrix = NULL ;
    scoreMatrix = (struct _poaAlignScore **)malloc(sizeof(struct _poaAlignScore *) * (len+1) ) ; // row is the input sequence
    for (i = 0 ; i <= len ; ++i)
      scoreMatrix[i] = (struct _poaAlignScore *)calloc(size, sizeof(struct _poaAlignScore)) ; // column is the POA     
    scoreMatrix[_sourceId][0].score = 0 ;
    scoreMatrix[_sourceId][0].prev.first = scoreMatrix[_sourceId][0].prev.second = -1 ;

    for (i = 0 ; i <= len ; ++i)
    {
      for (j = 1 ; j < size - 1 ; ++j) // aftter top sort, 0 is source, size-1 is sink
      {
        int nid = sortNodes[j] ; 
        int prevSize = _nodes[ nid ].prev.size() ;
        int match = _matScore ;
        if (i > 0 && seq[i - 1] != _nodes[ nid ].c)
          match = _misScore ;
        for (k = 0 ; k < prevSize ; ++k)
        {
          // Deletion to the graph
          int prevj = _nodes[ nid ].prev[k] ;
          int score = scoreMatrix[i][prevj].score + _delScore ; 
          std::pair<int, int> prev ;
          prev.first = i ;
          prev.second = prevj ;
          if (i > 0)
          {
            // match/mismatch
            if (scoreMatrix[i - 1][prevj].score + match > score)
            {
              score = scoreMatrix[i - 1][prevj].score + match ;
              prev.first = i - 1 ;
              prev.second = prevj ;
            }

            // Insertion to the graph
            if (i > 0 && scoreMatrix[i - 1][ nid ].score + _insScore > score)
            {
              score = scoreMatrix[i - 1][nid].score + _insScore ;
              prev.first = i - 1 ;
              prev.second = nid ;
            }
          }
          if (k == 0 || score > scoreMatrix[i][nid].score)
          {
            scoreMatrix[i][nid].score = score ;
            scoreMatrix[i][nid].prev = prev ;
          }
        }
      }
    }

    // Find the good sink.
    std::vector<int> &sinks = _nodes[_sinkId].prev ;
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
  // Note that the sequence index in path(.first) has padding, i.e. the index 0 is for source
  void Add(char *seq, int len, std::vector< std::pair<int, int> > path)
  {
    int i, j ; 
    int nid ; //tracking the current node id to add the edge to next node
    int p ;
    int pathSize = path.size() ; 
    nid = _sourceId ;
    
    for (p = 1 ; p < pathSize ; )
    {
      i = path[p].first ; // notice in the path i==0 is a padding
      j = path[p].second ;
      if (i != path[p - 1].first 
          && j != path[p - 1].second) // match and mismatch
      {
        if (seq[i - 1] == _nodes[j].c)
        {
          AddEdge(nid, j, 1) ;
          nid = j ;
        }
        else
        {
          int nextnid = AddNode(seq[i - 1]) ;
          AddEdge(nid, nextnid, 1) ;
          
          // Add an edge placeholder copying j's next 
          // This trick might be useful when having consecutive mismatches,
          //   this allows future alignment transit back to the backbone.
          // Maybe not necessary?
          /*int jNextSize = _nodes[j].next.size() ;
          for (int jnext = 0 ; jnext < jNextSize ; ++jnext)
            AddEdge(nextnid, _nodes[j].next[jnext].first, 0) ;*/
          nid = nextnid ;
        }
        ++p ;
      }
      else if (i == path[p - 1].first) // deletion
      {
        // nid stays the same, so it will directly point to the next match/mismatch node
        int k ;
        for (k = p + 1 ; k < pathSize ; ++k)
          if (path[k].first != path[p - 1].first)
            break ;
        p = k ; 
      }
      else // if (path[p].second == path[p - 1].second). insertion
      {
        int k ;
        for (k = p ; k < pathSize ; ++k)
        {
          if (path[k].second != path[p - 1].second)
            break ;
          int nextnid = AddNode(seq[path[k].first - 1]) ;
          AddEdge(nid, nextnid, 1) ;
          nid = nextnid ;
        }
        p = k ;
      }
    }

    // Don't forget to connect to sink
    AddEdge(nid, _sinkId, 1) ;
  }

  // In this case, we need to do the alignment
  int Add(char *seq, int len)
  {
    std::vector< std::pair<int, int> > path ;
    int ret = Align(seq, len, path) ;
    Add(seq, len, path) ;
    return ret ;
  }

  // Allocate the memory and return the most likely consensus sequences
  char *Consensus(int minWeight = 1)
  {
    int i, j ;
    
    int nodeCnt = _nodes.size() ;
    
    double *nodeScore ; // first: total weight, second : path length (number of nodes visited, inclusive)
    int *nodeNext ; // the selected succssor for each node 
    nodeScore = (double *)malloc(sizeof(*nodeScore) * nodeCnt) ;
    nodeNext = (int *)malloc(sizeof(*nodeNext) * nodeCnt) ;
  
    std::vector<int> sortNodes ;
    TopologicalSort(sortNodes) ;
    
    nodeScore[_sinkId] = 1 ; // likelihood
    memset(nodeNext, -1, sizeof(*nodeNext) * nodeCnt) ;
    for (i = nodeCnt - 1 ; i >= 0 ; --i)
    {
      int k = sortNodes[i] ;
      int nextSize = _nodes[k].next.size() ;
      double lsum = 0 ;
      for (j = 0 ; j < nextSize ; ++j)
        lsum += _nodes[k].next[j].second ;
      lsum = log(lsum) ;
      
      for (j = 0 ; j < nextSize ; ++j)
      {
        if (_nodes[k].next[j].second < minWeight)
          continue ;
        
        int nextnid = _nodes[k].next[j].first ;
        double testScore = log(_nodes[k].next[j].second) - lsum  + nodeScore[nextnid] ;
        if (nodeNext[k] == -1
            || testScore > nodeScore[k])
        {
          nodeScore[k] = testScore;
          nodeNext[k] = nextnid ;
        }
      }
    }

    int len = 0 ;
    i = _sourceId ;
    while (i != _sinkId)
    {
      i = nodeNext[i] ;
      ++len ;
    }
    --len ; // exclude sink node 

    char *consensus = (char *)malloc(sizeof(char) * (len + 1)) ;
    i = nodeNext[_sourceId] ;
    j = 0 ;
    while (i != _sinkId)
    {
      consensus[j] = _nodes[i].c ;
      i = nodeNext[i] ;
      ++j ;
    }
    consensus[len] = '\0' ;

    free(nodeScore) ;
    free(nodeNext) ;
    return consensus ;
  }

  void VisualizePOA()
  {
    int i, j ;
    int nodeCnt = _nodes.size() ;
    for (i = 0 ; i < nodeCnt ; ++i)
    {
      printf("%d: ", i) ;
      int nextSize = _nodes[i].next.size() ;
      for (j = 0 ; j < nextSize ; ++j)
        printf("%d(%d) ", _nodes[i].next[j].first, _nodes[i].next[j].second) ;
      printf("\n") ;
    }
  }

  void VisualizeAlignment(char *seq, int len, const std::vector< std::pair<int, int> > &path)
  {
    int i ;
    int p ;
    int width = 60 ;

    int pathSize = path.size() ;
    p = 1 ;
    while (p < pathSize)
    {
      for ( i = p ; i < p + width && i < pathSize ; ++i  )
      {
        if (path[i].second == path[i - 1].second)
          printf( "-" ) ;
        else
          printf( "%c", _nodes[ path[i].second ].c ) ;
      }
      printf( "\n" ) ;
      for ( i = p ; i < p + width && i < pathSize ; ++i )
      {
        if ( path[i].first != path[i - 1].first 
            && path[i].second != path[i - 1].second 
            && seq[ path[i].first - 1] == _nodes[ path[i].second ].c)
          printf( "|" ) ;
        else
          printf( " " ) ;
      }
      printf( "\n" ) ;
      for ( i = p ; i < p + width && i < pathSize ; ++i  )
      {
        if (path[i].first == path[i - 1].first)
          printf("-") ;
        else
          printf( "%c", seq[ path[i].first - 1] ) ;
      }
      printf( "\n\n" ) ;
      p = i ;
    }
  }
} ;

#endif
