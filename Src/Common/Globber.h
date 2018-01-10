//
//  Globber.h
//  Dasher
//
//  Created by Alan Lawrence on 21/9/11.
//  Copyright 2011 Cambridge University. All rights reserved.
//

#ifndef __GLOBBER_H__
#define __GLOBBER_H__

#include "../DasherCore/AbstractXMLParser.h"

void globScan(AbstractParser *parser,
              const char **userPaths,
              const char **systemPaths);
void globScan(AbstractParser *parser, 
              const char **usrPaths, 
              const char **sysPaths, 
              int(*error_callback)(const char *,int));

#endif
