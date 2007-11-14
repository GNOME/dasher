#ifndef __GameStatistics_h__
#define __GameStatistics_h__

// Code examples modified from C++ Cookbook O'Reilly.
// Code use falls under fair use as dicussed in the book

#include <numeric>
#include <cmath>
#include <algorithm>
#include <functional>

template<class T>
T nthPower(T x, int n)
{
  T ret = x;
  for (int i=1; i < n; ++i)
    ret*=x;
  return ret;
}

template<class T>
struct SumDiffNthPower 
{
  SumDiffNthPower(T x, int n) : m_mean(x), m_n(n) {}
  T operator()(T sum, T current)
  {
    return sum+nthPower(current-m_mean, m_n);
  }
  T m_mean;
  int m_n;
};

template<class T, class Iter_T>
  T nthMoment(int n, Iter_T first, Iter_T last, T mean)
{
  size_t cnt = last - first;
  return std::accumulate(first, last, T(), SumDiffNthPower<T>(mean, n))/cnt;
}

template<class A, class B, class T>
struct MemberSumDiffNthPower 
{
  MemberSumDiffNthPower(A T::* pm, B x, int n) : m_mean(x), m_pm(pm), m_n(n) {}
  B operator()(B sum, T current)
  {
    return sum+nthPower(current.*m_pm-m_mean, m_n);
  }
  B m_mean;
  A T::* m_pm;
  int m_n;
};

template<class T, class Iter_T, class A, class B>
  B MemberNthMoment(int n, Iter_T first, Iter_T last, A T::* pmember, B mean)
{
  size_t cnt = last - first;
  return std::accumulate(first, last, B(), MemberSumDiffNthPower<A,B,T>(pmember,mean,n))/cnt;
}

#endif
