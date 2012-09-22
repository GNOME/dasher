// SimplePooledAlloc.h
//
// Copyright (c) 2005 David Ward

#ifndef __SimplePooledAlloc_h__
#define __SimplePooledAlloc_h__

// CSimplePooledAlloc allocates objects T in fixed-size blocks (specified) 
// Alloc returns a default-constructed T*
// Memory is only freed on destruction of the allocator

#include <cstddef>
#include <vector>

template<typename T>
class CSimplePooledAlloc {
public:
  // Construct with given block size
  CSimplePooledAlloc(std::size_t iBlockSize);

  ~CSimplePooledAlloc();

  // Return an uninitialized object
  T *Alloc();

private:
  class CPool {
  public:
    CPool(std::size_t iSize):m_iCurrent(0), m_iSize(iSize) {
      m_pData = new T[m_iSize];
    }
    ~CPool() {
      delete[]m_pData;
    }
    T *Alloc() const {
      if(m_iCurrent < m_iSize)
        return &m_pData[m_iCurrent++];
      return NULL;
    }
  private:
    mutable std::size_t m_iCurrent;
    std::size_t m_iSize;
    T *m_pData;
  };

  std::vector < CPool * >m_vPool;

  std::size_t m_iBlockSize;
  int m_iCurrent;
};

template < typename T > CSimplePooledAlloc < T >::CSimplePooledAlloc(std::size_t iSize):m_iBlockSize(iSize), m_iCurrent(0) {
  m_vPool.push_back(new CPool(m_iBlockSize));
}

template < typename T > CSimplePooledAlloc < T >::~CSimplePooledAlloc() {
  for(std::size_t i = 0; i < m_vPool.size(); i++)
    delete m_vPool[i];
}

template < typename T > T * CSimplePooledAlloc < T >::Alloc() {
  T *p = m_vPool[m_iCurrent]->Alloc();
  if(p)
    return p;
  m_vPool.push_back(new CPool(m_iBlockSize));
  m_iCurrent++;
  return m_vPool.back()->Alloc();
}

#endif // __include__
