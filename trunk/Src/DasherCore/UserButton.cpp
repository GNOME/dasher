#include "TimeSpan.h"
#include "UserButton.h"

#include <sstream>

CUserButton::CUserButton(int iId, int iType, int iEffect) {
  m_iId = iId;
  m_iType = iType;
  m_iEffect = iEffect;
  m_strStringTime = CTimeSpan::GetTimeStamp();
}

std::string CUserButton::GetXML(const std::string& strPrefix) {
  std::stringstream ssResult;

  ssResult << strPrefix << "<Button>" << std::endl;

  ssResult << strPrefix << "\t<Id>" << m_iId << "</Id>" << std::endl;
  ssResult << strPrefix << "\t<Type>" << m_iType << "</Type>" << std::endl;
  ssResult << strPrefix << "\t<Effect>" << m_iEffect << "</Effect>" << std::endl;
  ssResult << strPrefix << "\t<Time>" << m_strStringTime << "</Time>" << std::endl;

  ssResult << strPrefix << "</Button>" << std::endl;

  return ssResult.str();
}
