#include "Messages.h"
#include <string.h>
#include <vector>
#include <stdarg.h>
#include <stdio.h>

using std::vector;

void CMessageDisplay::FormatMessageWithString(const char *fmt, const char *str) {
  char *buf(new char[strlen(fmt)+strlen(str)]);
  sprintf(buf, fmt, str);
  Message(buf, true);
  delete[] buf;
}

void CMessageDisplay::FormatMessageWith2Strings(const char *fmt, const char *str1, const char *str2) {
  char *buf(new char[strlen(fmt)+strlen(str1)+strlen(str2)]);
  sprintf(buf, fmt, str1, str2);
  Message(buf,true);
  delete[] buf;
}

  //The following implements a varargs version of the above,
  // dynamically allocating enough storage for the formatted string
  // using snprintf. However, this doesn't work on Solaris,
  // hence commenting out.

  //Note: vector is guaranteed to store elements contiguously.
  // C++98 did not guarantee this, but this was corrected in a 2003
  // technical corrigendum. As Bjarne Stroustrup says, 
  // "this was always the intent and all implementations always did it that way"
  /*vector<char> buf;
  for (int len = strlen(fmt)+1024; ;) {
    buf.resize(len);
    va_list args;
    va_start(args,fmt);
    int res = vsnprintf(&buf[0], len, fmt, args);
    va_end(args);
    if (res>=0 && res<len) {
      //ok, buf big enough, now contains string
      Message(&buf[0], true);
      return;
    }
    if (res<0) {
      //on windows, returns -1 for "buffer not big enough" => double size & retry.
      // However, on linux, -1 indicates "some other error".
      // So make sure we don't infinite loop but instead break out somehow...
      if (len*=2 > 1<<16) {
        printf("Could not allocate big enough buffer, or other error, when trying to print:\n");
        va_list args2;
        va_start(args2,fmt);
        vprintf(fmt,args2);
        va_end(args2);
        return; //exit loop + function, no call to Message()
      }
    } else len = res+1; //that identifies necessary size of buffer
  }*/
