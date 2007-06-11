// int32.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __int32_h__
#define __int32_h__

#include "../myassert.h"
#include "int.h"

// Get rid of annoying namespace pollution
#undef max
#undef min

#include <limits>

#ifdef _DEBUG

class Cint64;

#ifdef _MSC_VER
#if _MSC_VER == 1300
#define NON_STANDARD_LL
#endif
#if _WIN32_WCE
#define NON_STANDARD_LL
#endif

#endif

#ifdef NON_STANDARD_LL
const int64 int64_0xffffffff80000000LL = 0xffffffff80000000i64;
const uint64 uint64_0xffffffff00000000ULL = 0xffffffff80000000ui64;
#else
const int64 int64_0xffffffff80000000LL = 0xffffffff80000000LL;
const uint64 uint64_0xffffffff00000000ULL = 0xffffffff00000000ULL;
#endif

class Cint32 {
 public:
  static int32 Max() {
    return std::numeric_limits<int32>::max();
  }
  static int32 Min() {
    return std::numeric_limits<int32>::min();
  }

  Cint32() {
  }

  Cint32(int32 i):m_i(i) {
  }

  Cint32(int64 i);

  Cint32(double d) {
    DASHER_ASSERT(d <= double(Cint32::Max()));
    DASHER_ASSERT(d >= double(Cint32::Min()));
    
    m_i = int32(d);
  }

  Cint32(Cint64 i);

  operator  double () const {
    return double (m_i);
  }
  operator  long () const {
    return long (m_i);
  }
  operator  int64() const {
    return m_i;
  }
  operator  int32() const {
    return m_i;
  }
  Cint32 & operator+=(Cint32 rhs) {
    if(!AreDifferentSigns(m_i, rhs.m_i)) {
      // 2-ve or 2-ve
      if(rhs.m_i < 0) {
        // 2-ve
        if(m_i < Min() - rhs.m_i) {
          DASHER_ASSERT(0);
        }
        // ok
      }
      else {
        // 2+ve
        if(Max() - m_i < rhs.m_i) {
          DASHER_ASSERT(0);
        }
        // ok
      }
    }
    m_i += rhs.m_i;
    return *this;
  }

  Cint32 & operator-=(Cint32 rhs) {
    if(AreDifferentSigns(m_i, rhs.m_i)) {
      // 1-ve, 1+ve
      if(m_i >= 0) {
        if(m_i > Max() + rhs.m_i) {
          DASHER_ASSERT(0);
        }
      }
      else {
        // lhs -ve
        if(m_i < Min() + rhs.m_i)
          DASHER_ASSERT(0);
        // ok
      }
    }
    m_i -= rhs.m_i;
    return *this;
  }

  Cint32 & operator*=(Cint32 rhs) {

    int64 tmp = (int64) m_i * (int64) rhs.m_i;

    // upper 33 bits must be equal
    if((tmp & int64_0xffffffff80000000LL) == 0 || (tmp & int64_0xffffffff80000000LL) == int64_0xffffffff80000000LL) {
      m_i = (int32) tmp;
      return *this;
    }

    DASHER_ASSERT(0);
    return *this;
  }

  Cint32 & operator/=(Cint32 rhs) {
    if(rhs.m_i == 0)
      DASHER_ASSERT(0);

    // corner case where this = Min and rhs = -1
    if(m_i == Min() && rhs.m_i == -1)
      DASHER_ASSERT(0);

    m_i /= rhs.m_i;
    return *this;
  }

  bool operator!=(Cint32 rhs) const {
    return m_i != rhs.m_i;
  }
  bool operator<=(int rhs)const {
    return m_i <= rhs;
  }
  bool operator>=(int rhs)const {
    return m_i >= rhs;
  }
  bool operator<(int rhs)const {
    return m_i < rhs;
  }
  bool operator>(int rhs)const {
    return m_i > rhs;
  }
  bool operator<(Cint32 rhs) const {
    return m_i < rhs.m_i;
  }
  bool operator>(Cint32 rhs) const {
    return m_i > rhs.m_i;
  }
  bool operator<=(Cint32 rhs) const {
    return m_i <= rhs.m_i;
  }
  bool operator>=(Cint32 rhs) const {
    return m_i >= rhs.m_i;
  }
 private:
  int32 m_i;
};

inline Cint32 operator +(Cint32 lhs, Cint32 rhs) {
  lhs += rhs;
  return lhs;
}

inline Cint32 operator +(Cint32 lhs, int rhs) {
  lhs += Cint32(rhs);
  return lhs;
}

inline double operator +(double lhs, Cint32 rhs) {
  return lhs + double (rhs);
}

inline double operator +(Cint32 lhs, double rhs) {
  return double (lhs) + rhs;
}

inline Cint32 operator +(int lhs, Cint32 rhs) {
  rhs += Cint32(lhs);
  return rhs;
}

inline Cint32 operator -(Cint32 lhs, Cint32 rhs) {
  lhs -= rhs;
  return lhs;
}

inline double operator -(double lhs, Cint32 rhs) {
  return lhs - double (rhs);
}

inline double operator -(Cint32 lhs, double rhs) {
  return double (lhs) - rhs;
}

inline Cint32 operator -(Cint32 lhs, int rhs) {
  lhs -= Cint32(rhs);
  return lhs;
}

inline Cint32 operator -(int lhs, Cint32 rhs) {
  Cint32 temp = lhs;
  temp -= rhs;
  return temp;
}

inline Cint32 operator *(Cint32 lhs, int rhs) {
  lhs *= Cint32(rhs);
  return lhs;
}

inline Cint32 operator *(int lhs, Cint32 rhs) {
  Cint32 temp = lhs;
  temp *= rhs;
  return temp;
}

inline Cint32 operator *(Cint32 lhs, Cint32 rhs) {
  lhs *= rhs;
  return lhs;
}

/*inline Cint32 operator *(int lhs, Cint32 rhs)
{
	rhs*=Cint32(lhs);
	return rhs;
}
*/

inline double operator *(double lhs, Cint32 rhs) {
  return lhs * double (rhs);
}

inline double operator *(Cint32 lhs, double rhs) {
  return rhs * double (lhs);
}

inline Cint32 operator /(Cint32 lhs, int rhs) {
  lhs /= Cint32(rhs);
  return lhs;
}

inline Cint32 operator /(Cint32 lhs, Cint32 rhs) {
  lhs /= rhs;
  return lhs;
}

inline Cint32 operator /(int lhs, Cint32 rhs) {
  Cint32 temp(lhs);
  temp /= rhs;
  return temp;
}

inline double operator /(Cint32 lhs, double rhs) {
  return double (lhs) / rhs;
}

inline double operator /(double lhs, Cint32 rhs) {
  return lhs / double (rhs);
}

inline bool operator >(double lhs, Cint32 rhs) {
  return lhs > double (rhs);
}

inline bool operator >(long lhs, Cint32 rhs) {
  return lhs > long (rhs);
}

inline bool operator <(long lhs, Cint32 rhs) {
  return lhs < long (rhs);
}

#include "int64.h"

inline Cint32::Cint32(Cint64 i) {
  if((int64) i > int64(Cint32::Max()))
    DASHER_ASSERT(0);
  if((int64) i < int64(Cint32::Min()))
    DASHER_ASSERT(0);
  m_i = int32(i);
}


inline Cint32 abs(Cint32 t) {
  return Cint32(abs(int32(t)));
}

#else

typedef int32 Cint32;

#endif // _DEBUG

#endif // __include__
