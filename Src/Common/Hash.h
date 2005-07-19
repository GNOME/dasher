// Hash.h
//
// Copyright (c) 2005 David Ward

#ifndef __Common_Hash_h__
#define __Common_Hash_h__

// String hash function

inline size_t hash_string(const char *s) {
  register size_t i;
  for(i = 0; *s; s++) {
    i *= 16777619;
    i ^= *s;
  }
  return i;

}

#endif
