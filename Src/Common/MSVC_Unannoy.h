#ifndef __MSVC_Unannoy_h__
#define __MSVC_Unannoy_h__

// Sorry about this frequently included header non-VC++ users.
// It shouldn't do any harm.

// Warning 4786 is so annoying if you have VC++ 6.
// It produces *pages* of complaints if you use certain STL headers.
// The warning just means that VC++ is unable to produce certain debug
// information - there is nothing wrong with the code. IAM 08/2002
#ifdef _MSC_VER
#pragma warning(disable:4786)
#pragma warning(disable:4018)

#pragma warning(disable:4996)   // warning C4996: 'fopen' was declared deprecated
#endif

#endif /* #ifndef __MSVC_Unannoy_h__ */
