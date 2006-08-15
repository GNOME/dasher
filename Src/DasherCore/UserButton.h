#ifndef __UserButton_h__
#define __UserButton_h__

#include "FileLogger.h"
#include <string>
#include <vector>

extern CFileLogger* g_pLogger;

class CUserButton;

typedef std::vector<CUserButton>               VECTOR_USER_BUTTON;
typedef std::vector<CUserButton>::iterator     VECTOR_USER_BUTTON_ITER;
typedef std::vector<CUserButton*>              VECTOR_USER_BUTTON_PTR;
typedef std::vector<CUserButton*>::iterator    VECTOR_USER_BUTTON_PTR_ITER;

class CUserButton {
 public:
  CUserButton(int iId);

  std::string GetXML(const std::string& strPrefix = "");
    
 protected:
  int m_iId;
  std::string m_strStringTime;
};

#endif
