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

  // Useful for control semi-global alignment 
  int _delStartScore ; // score of delete first part of the POA
  int _delEndScore ; // score of deletion last part of the POA.

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

  // return sign of 1's - 2's
  // Need to adjust the likelihood because it biased towards shorter paths
  int CompLikelihoodAdjustedByLength(double l1, double supportWeight1, int len1, double l2, double supportWeight2, int len2)
  {
    int diff = len1 - len2 ;
    if (diff == 0 
        || (diff > 0 && l1 > l2)
        || (diff < 0 && l1 < l2))
    {
      if (l1 != l2)
        return (l1 - l2 > 0) ? 1 : -1 ;
      else
        return 0 ;
    }
    double adjustL1 = l1 ;
    double adjustL2 = l2 ;
    if (len1 < len2)
      // Need adjuaces(decrease) 1's likelihood
      adjustL1 += (len2 - len1) * log((double)supportWeight1) ;
    else
      adjustL2 += (len1 - len2) * log((double)supportWeight2) ;

    if (adjustL1 != adjustL2)
      return (adjustL1 > adjustL2) ? 1 : -1 ;
    else
      return 0;
  }
public:
  POA() : _sourceId(0), _sinkId(1), _effectiveIdStart(2) 
  {
    _matScore = 0 ;
    _misScore = -2 ;
    _insScore = -4 ;
    _delScore = _delStartScore = _delEndScore = -4 ;
    
    AddNode('\0') ; // source
    AddNode('\0') ; // sink
  }

  ~POA() {}

  void SetMatchScore(int s)
  {
    _matScore = s ;
  }

  void SetMismatchScore(int s)
  {
    _misScore = s ;
  }

  void SetInsScore(int s)
  {
    _insScore = s ;
  }

  void SetDelScore(int s)
  {
    _delScore = s ;
  }

  void SetDelStartScore(int s)
  {
    _delStartScore = s ;
  }

  void SetDelEndScore(int s)
  {
    _delEndScore = s ;
  }

  void Init(char *seq, size_t len)
  {
    size_t i ;
    int nid = _sourceId ;
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
          if (i == 0 && _delStartScore != _delScore)
            score = scoreMatrix[i][prevj].score + _delStartScore ;
          else if (i == len && _delEndScore != _delScore)
            score = scoreMatrix[i][prevj].score + _delEndScore ;
          
          std::pair<int, int> prev ;
          prev.first = i ;
          prev.second = prevj ;
          if (i > 0)
          {
            // match/mismatch
            if (scoreMatrix[i - 1][prevj].score + match >= score)
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
  char *Consensus(double outMinWeight = 0, bool lengthAdjustment = false)
  {
    int i, j ;
    
    int nodeCnt = _nodes.size() ;
    
    double *nodeScore ; // first: total weight, second : path length (number of nodes visited, inclusive)
    int *nodeNext ; // the selected succssor for each node 
    int *nodeDistToSink ; // the nodes distance to sink based on the path from adjusted max likelihood

    nodeScore = (double *)malloc(sizeof(*nodeScore) * nodeCnt) ;
    nodeNext = (int *)malloc(sizeof(*nodeNext) * nodeCnt) ;
    nodeDistToSink = (int *)malloc(sizeof(*nodeDistToSink) * nodeCnt); 
  
    std::vector<int> sortNodes ;
    TopologicalSort(sortNodes) ;
    
    nodeScore[_sinkId] = 1 ; // likelihood
    nodeDistToSink[_sinkId] = 0 ;
    memset(nodeNext, -1, sizeof(*nodeNext) * nodeCnt) ;
    for (i = nodeCnt - 1 ; i >= 0 ; --i)
    {
      int k = sortNodes[i] ;
      int nextSize = _nodes[k].next.size() ;
      double sum = 0 ;
      double lsum = 0 ;
      for (j = 0 ; j < nextSize ; ++j)
        sum += _nodes[k].next[j].second ;
      lsum = log(sum) ;
      
      int bestj = 0 ;
      for (j = 0 ; j < nextSize ; ++j)
      {
        if (_nodes[k].next[j].second < outMinWeight * sum)
          continue ;
        
        int nextnid = _nodes[k].next[j].first ;
        double testScore = log(_nodes[k].next[j].second) - lsum  + nodeScore[nextnid] ;
        //printf("%d:%d=>%d,%lf,%d,%d\n", i, k, nextnid, testScore, nodeDistToSink[nextnid], 
        //    _nodes[k].next[j].second) ;
        if (nodeNext[k] == -1
            || (lengthAdjustment == false && testScore > nodeScore[k])
            || (lengthAdjustment == true 
              && CompLikelihoodAdjustedByLength(
                testScore, _nodes[k].next[j].second / (double)sum, 1 + nodeDistToSink[nextnid],
                nodeScore[k], _nodes[k].next[ bestj ].second / (double)sum, nodeDistToSink[k]) > 0))
        {
          bestj = j ;
          nodeScore[k] = testScore;
          nodeNext[k] = nextnid ;
          nodeDistToSink[k] = nodeDistToSink[nextnid] + 1 ;
        }
      }
      //printf("%d:%d(%c)=>%d\n", i, k, _nodes[k].c, nodeNext[k]) ;
    }

    //i = _sourceId ;
    //for (j = 0 ; j < _nodes[i].next.size() ; ++j)
    //  printf("%d %d\n", _nodes[i].next[j].first, nodeDistToSink[ _nodes[i].next[j].first ] ) ;

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
