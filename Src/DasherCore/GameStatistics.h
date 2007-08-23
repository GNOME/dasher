#ifndef __GameStatistics_h__
#define __GameStatistics_h__

// Code example from C++ Cookbook O'Reilly.
// Code use falls under fair use as dicussed in the book

#include <numeric>
#include <cmath>
#include <algorithm>
#include <functional>

template<int N, class T>
T nthPower(T x)
{
  T ret = x;
  for (int i=1; i< N; ++i)
    ret*=x;
  return ret;
}

template<class T, int N>
struct SumDiffNthPower 
{
  SumDiffNthPower(T x) : m_mean(x) {}
  T operator()(T sum, T current)
  {
    return sum+nthPower<N>(current-m_mean);
  }
  T m_mean;
};

template<class T, int N, class Iter_T>
  T nthMoment(Iter_T first, Iter_T last, T mean)
{
  size_t cnt = std::distance(first, last);
  return std::accumulate(first, last, T(), SumDiffNthPower<T,N>(mean))/cnt;
}
#endif
