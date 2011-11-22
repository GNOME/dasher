//
//  Globber.cpp
//  Dasher
//
//  Created by Alan Lawrence on 21/9/11.
//  Copyright 2011 Cambridge University. All rights reserved.
//

#include "Globber.h"
#include <glob.h>
#include <string.h>

void globScan(AbstractParser *parser, const char **usrPaths, const char **sysPaths) {
  int flags = GLOB_MARK | GLOB_NOSORT;
  glob_t info;
  int numUser = 0;
  while (*usrPaths) {
    glob(*usrPaths++, flags, NULL, &info); //NULL error function
    flags |= GLOB_APPEND;
  }
  numUser = info.gl_pathc;
  while (*sysPaths) {
    glob(*sysPaths++, flags, NULL, &info);
    flags |= GLOB_APPEND;
  }
  
  if (info.gl_pathc) {
    //user paths first
    for (char **fname = info.gl_pathv; *fname; fname++, numUser=(numUser>0 ? numUser-1 : 0)) {
      if ((*fname)[strlen(*fname)-1]=='/') continue; //directories were marked by GLOB_MARK
      parser->ParseFile(*fname, numUser>0);
    }
  }
  globfree(&info);
}
