 #include "MockFileWordGenerator.h"


bool MockFileWordGenerator::Generate() { 
  if(!m_sFileHandle.is_open()) {
    m_sFileHandle.open(m_sPath.c_str());
  }
  if(m_sFileHandle.fail()) {
    throw std::runtime_error("File: " + m_sPath + " cannot be read.");
  }
  
  m_uiPos = 0;
  m_sGeneratedString.clear();
  
  if(m_sFileHandle.eof()) {
    return false;
  } else if(m_sFileHandle.good()) {
    std::getline(m_sFileHandle, m_sGeneratedString);
    return true;
  } else {
    return false;
  }
}

std::string MockFileWordGenerator::GetPath() {
  return m_sPath; 
}
std::string MockFileWordGenerator::GetFilename() {
  return m_sPath.substr(m_sPath.rfind("/")+1);
}

std::string MockFileWordGenerator::GetNextWord() {
  if(m_uiPos >= m_sGeneratedString.length()) { 
    // Attempt to reload the buffer.
    if(!Generate()) return "";
  } 
  
  size_t found = m_sGeneratedString.substr(m_uiPos).find(" ");
  std::string result;
  
    // If there are no space characters.
  if(found != string::npos) 
  {
    result = m_sGeneratedString.substr(m_uiPos, found);

    m_uiPos += (found + 1);
  } else {
    result = m_sGeneratedString.substr(m_uiPos);
  };
  
  return result;
}


