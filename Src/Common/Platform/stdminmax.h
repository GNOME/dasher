// stdminmax.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __Platform_stdminmax__
#define __Platform_stdminmax__

#if _MSC_VER < 1300

namespace std {
  template < typename T > inline T min(T t1, T t2) {
    return std::_cpp_min(t1, t2);
  } template < typename T > inline T max(T t1, T t2) {
    return std::_cpp_max(t1, t2);
  }

};
#endif

#endif // __Platform_stdminmax__
