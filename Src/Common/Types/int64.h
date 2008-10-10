// int64.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __int64_h__
#define __int64_h__

#include "int.h"

//#ifdef _MSC_VER

// Get rid of annoying namespace pollution
#undef max
#undef min

#include <limits>
//#endif

#include "../myassert.h"

#ifdef _DEBUG

class Cint32;

class Cint64 {

public:

  int64 Max() {
    return std::numeric_limits<int64>::max();
  }
  int64 Min() {
    return std::numeric_limits<int64>::min();
  }

  Cint64() {
  }
  Cint64(int64 i):m_i(i) {
  }
  Cint64(Cint32 i);

  Cint64(double d) {
    DASHER_ASSERT(d <= double (Cint64::Max()));
    DASHER_ASSERT(d >= double (Cint64::Min()));
      
    m_i = int64(d);
  }
  
  Cint64(int32 i):m_i(i) {
  }
  Cint64(uint32 i):m_i(i) {
  }

  operator  double () const {
    return double (m_i);
  }
  operator  int64() const {
    return m_i;
  }
  operator  int32() const;
  operator  uint32() const;

  Cint64 & operator+=(Cint64 rhs) {
    // Sorry about the rather unweildy conditional here, but it avoids a lot of pointless checks in release code.
    DASHER_ASSERT( AreDifferentSigns(m_i, rhs.m_i) || (((rhs.m_i > 0) || (m_i >= Min() - rhs.m_i)) && ((rhs.m_i < 0) || (Max() - m_i >= rhs.m_i))));

    //if(!AreDifferentSigns(m_i, rhs.m_i)) {
    //  // 2-ve 2+ve
    //  if(rhs.m_i < 0) {
    //    // 2-ve
    //    if(m_i < Min() - rhs.m_i) {
    //      DASHER_ASSERT(0);
    //    }
    //    // ok
    //  }
    //  else {
    //    // 2+ve
    //    if(Max() - m_i < rhs.m_i) {
    //      DASHER_ASSERT(0);
    //    }
    //    // ok
    //  }
    //}

    m_i += rhs.m_i;
    return *this;
  }

  Cint64 & operator-=(Cint64 rhs) {
    DASHER_ASSERT( !AreDifferentSigns(m_i, rhs.m_i) || (((m_i < 0) || (m_i <= Max() + rhs.m_i)) && ((m_i >= 0) || (m_i >= Min() + rhs.m_i))));

    //if(AreDifferentSigns(m_i, rhs.m_i)) {
    //  // 1+ve 1-ve

    //  if(m_i >= 0) {
    //    if(m_i > Max() + rhs.m_i) {
    //      DASHER_ASSERT(0);
    //    }
    //  }
    //  else {
    //    if(m_i < Min() + rhs.m_i) {
    //      DASHER_ASSERT(0);
    //    }

    //  }
    //}
    m_i -= rhs.m_i;
    return *this;
  }

  Cint64 & operator*=(Cint64 rhs) {
    if(m_i == 0 || rhs.m_i == 0) {
      m_i = 0;
      return *this;
    }
     
    if(!AreDifferentSigns(m_i, rhs.m_i)) {
      // 2 -ve, 2 +ve => result +ve
      if(m_i > 0) {
        // 2+ve
        if(Max() / m_i < rhs.m_i)
          DASHER_ASSERT(0);
      }
      else {
        // 2-ve
        if(m_i == Min() || rhs.m_i == Min())
          DASHER_ASSERT(0);
        if(Max() / (-m_i) < (-rhs.m_i))
          DASHER_ASSERT(0);
      }
    }
    else {
      // lhs * rhs < Min => overflow
      if(m_i < 0) {
        if(m_i < Min() / rhs.m_i)
          DASHER_ASSERT(0);
      }
      else {
        if(rhs.m_i < Min() / m_i)
          DASHER_ASSERT(0);
      }
    }

    //ok
    m_i *= rhs.m_i;
    return *this;
  }

  Cint64 & operator/=(Cint64 rhs) {
    if(rhs.m_i == 0)
      DASHER_ASSERT(0);

    // edge case where lhs = Min and rhs = -1
    if(m_i == Min() && rhs.m_i == -1)
      DASHER_ASSERT(0);

    m_i /= rhs.m_i;
    return *this;
  }

  bool operator!=(Cint64 rhs) const {
    return m_i != rhs.m_i;
  }
  bool operator==(Cint64 rhs) const {
    return m_i == rhs.m_i;
  }
  bool operator<=(int32 rhs) const {
    return m_i <= rhs;
  }
  bool operator>=(int32 rhs) const {
    return m_i >= rhs;
  }
  bool operator<(int32 rhs) const {
    return m_i < rhs;
  }
  bool operator>(int32 rhs) const {
    return m_i > rhs;
  }
  bool operator<(Cint64 rhs) const {
    return m_i < rhs.m_i;
  }
  bool operator>(Cint64 rhs) const {
    return m_i > rhs.m_i;
  }
  bool operator<=(Cint64 rhs) const {
    return m_i <= rhs.m_i;
  }
  bool operator>=(Cint64 rhs) const {
    return m_i >= rhs.m_i;
  }
 private:
    int64 m_i;
};

#include "int32.h"

inline Cint64::Cint64(Cint32 i):m_i(int64(i)) {
}

inline Cint64::operator  int32() const {
  if(m_i > Cint32::Max())
    DASHER_ASSERT(0);
  if(m_i < Cint32::Min())
    DASHER_ASSERT(0);

  return (int32) m_i;
}

inline Cint64::operator  uint32() const {
  return (uint32) m_i;
}

inline Cint64 operator +(Cint64 lhs, Cint64 rhs) {
  lhs += rhs;
  return lhs;
}

inline Cint64 operator +(Cint64 lhs, int32 rhs) {
  lhs += Cint64(rhs);
  return lhs;
}

inline double operator +(double lhs, Cint64 rhs) {
  return lhs + double(rhs);
}

inline double operator +(Cint64 lhs, double rhs) {
  return double(lhs) + rhs;
}

inline Cint64 operator +(Cint32 lhs, Cint64 rhs) {
  rhs += Cint64(lhs);
  return rhs;
}

inline Cint64 operator +(Cint64 lhs, Cint32 rhs) {
  lhs += Cint64(rhs);
  return lhs;
}

inline Cint64 operator +(int32 lhs, Cint64 rhs) {
  Cint64 temp = lhs;
  temp += rhs;
  return temp;
}

inline Cint64 operator -(Cint64 lhs, Cint64 rhs) {
  lhs -= rhs;
  return lhs;
}

inline Cint64 operator -(Cint32 lhs, Cint64 rhs) {
  Cint64 temp(lhs);
  temp -= rhs;
  return temp;
}

inline Cint64 operator -(Cint64 rhs) {
  Cint64 temp = 0;
  temp -= rhs;
  return temp;
}

inline double operator -(double lhs, Cint64 rhs) {
  return lhs - double (rhs);
}

inline double operator -(Cint64 lhs, double rhs) {
  return double (lhs) - rhs;
}

inline Cint64 operator -(Cint64 lhs, int32 rhs) {
  lhs -= Cint64(rhs);
  return lhs;
}

inline Cint64 operator -(int32 lhs, Cint64 rhs) {
  Cint64 temp = lhs;
  temp -= rhs;
  return temp;
}

inline Cint64 operator *(Cint64 lhs, int32 rhs) {
  lhs *= Cint64(rhs);
  return lhs;
}

inline Cint64 operator *(int32 lhs, Cint64 rhs) {
  Cint64 temp = lhs;
  temp *= rhs;
  return temp;
}

inline Cint64 operator *(Cint64 lhs, Cint64 rhs) {
  lhs *= rhs;
  return lhs;
}

inline Cint64 operator *(Cint64 lhs, Cint32 rhs) {
  lhs *= Cint64(rhs);
  return lhs;
}

inline Cint64 operator /(Cint64 lhs, Cint32 rhs) {
  lhs /= Cint64(rhs);
  return lhs;
}

inline Cint64 operator *(Cint32 lhs, Cint64 rhs) {
  rhs *= Cint64(lhs);
  return rhs;
}

/*inline Cint64 operator *(int32 lhs, Cint64 rhs)
{
	rhs*=Cint64(lhs);
	return rhs;
}
*/

inline double operator *(double lhs, Cint64 rhs) {
  return lhs * double (rhs);
}

inline double operator *(Cint64 lhs, double rhs) {
  return rhs * double (lhs);
}

inline Cint64 operator /(Cint64 lhs, int32 rhs) {
  lhs /= Cint64(rhs);
  return lhs;
}

inline Cint64 operator /(Cint32 lhs, Cint64 rhs) {
  Cint64 lhs2(lhs);
  lhs /= rhs;
  return lhs;
}

inline Cint64 operator /(Cint64 lhs, Cint64 rhs) {
  lhs /= rhs;
  return lhs;
}

inline Cint64 operator /(int32 lhs, Cint64 rhs) {
  Cint64 temp(lhs);
  temp /= rhs;
  return temp;
}

inline double operator /(Cint64 lhs, double rhs) {
  return double (lhs) / rhs;
}

inline double operator /(double lhs, Cint64 rhs) {
  return lhs / double (rhs);
}

inline bool operator >(double lhs, Cint64 rhs) {
  return lhs > double (rhs);
}

inline bool operator <(int32 lhs, Cint64 rhs) {
  return Cint64(lhs) < rhs;
}

inline bool operator >(int32 lhs, Cint64 rhs) {
  return Cint64(lhs) > rhs;
}

inline bool operator <(Cint32 lhs, Cint64 rhs) {
  return Cint64(lhs) < rhs;
}

inline bool operator >(Cint32 lhs, Cint64 rhs) {
  return Cint64(lhs) > rhs;
}


#else

typedef int64 Cint64;

#endif // _DEBUG

#endif // __include__
