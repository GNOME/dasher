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

/// \ingroup Logging
/// \{
class CUserButton {
 public:
  CUserButton(int iId, int iType, int iEffect);

  std::string GetXML(const std::string& strPrefix = "");

  int GetCount() {
    if(m_iType == 0)
      return 1;
    else
      return 0;
  }

 protected:
  int m_iId;
  int m_iType;
  int m_iEffect;
  std::string m_strStringTime;
};
/// \}

#endif
