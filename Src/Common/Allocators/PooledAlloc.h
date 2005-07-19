// PooledAlloc.h
//
// Copyright (c) 2005 David Ward

#ifndef __PooledAlloc_h__
#define __PooledAlloc_h__

// CPooledAlloc allocates objects T in fixed-size blocks (specified in the constructor) 
// Alloc returns an uninitialized T*
// Free returns an object to the pool

#include "SimplePooledAlloc.h"

/////////////////////////////////////////////////////////////////////////////

template<typename T> class CPooledAlloc {

public:

  // Construct with given block size
  CPooledAlloc(size_t iBlockSize);
  ~CPooledAlloc();

  // Return an uninitialized object
  T *Alloc();

  // Return an object to the pool
  void Free(T * pFree);

private:

  // Use simple pooled alloc for the blocked allocation
  CSimplePooledAlloc < T > m_Alloc;

  // The free list
  std::vector < T * >m_vpFree;

};

template<typename T> CPooledAlloc<T>::CPooledAlloc(size_t iSize):m_Alloc(iSize) {}

template<typename T> CPooledAlloc<T>::~CPooledAlloc() {}

template<typename T> T * CPooledAlloc < T >::Alloc() {
  if(m_vpFree.size() > 0) {
    T *pLast = m_vpFree.back();
    m_vpFree.pop_back();
    return pLast;
  }
  return m_Alloc.Alloc();
}

template<typename T> void CPooledAlloc<T>::Free(T *pFree) {
  m_vpFree.push_back(pFree);
}

#endif // __include__
