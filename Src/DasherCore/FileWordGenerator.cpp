#include "FileWordGenerator.h"

using namespace Dasher;

CFileWordGenerator::CFileWordGenerator(CMessageDisplay *pMsgs, const CAlphInfo *pAlph, const CAlphabetMap *pAlphMap)
  : CWordGeneratorBase(pAlph,pAlphMap), AbstractParser(pMsgs) {
}

bool CFileWordGenerator::Parse(const std::string &strDesc, istream &in, bool bUser) {
  //non-file streams not supported (yet)
  DASHER_ASSERT(false);
  return false;
}

bool CFileWordGenerator::ParseFile(const std::string &sPath, bool bUser) {
  if (bUser && !m_bAcceptUser) return false;
  m_sFileHandle.close();
  m_vLineIndices.clear();
  m_sPath=sPath;
  m_sFileHandle.open(m_sPath.c_str());
  if(m_sFileHandle.fail()) {
    m_sPath="";
    return false;
  }
  /*The hard way:
  m_vLineIndices.push_back(0);
  streampos pos=0;
  char buf[1024];
  for (int read = m_sFileHandle.read(buf,1024).gcount();;) {
    for (int i=0; i<read-1; i++) {
      if (buf[i]=='\r') {
        if (buf[i+1]=='\n') i++;
      } else if (buf[i]!='\n') continue;
      m_vLineIndices.push_back(pos+i);
    }
    if (read!=1024) break;
    //haven't checked final byte; might be \r and need \n from next read
    pos+=read-1;
    buf[0]=buf[read-1];
    read = m_sFileHandle.read(&buf[1],1023).gcount()+1;
  }
  */
  //The easy way - TODO will this work with Windows line endings?
  for (std::string buf; m_sFileHandle.good();) {
    streampos pos = m_sFileHandle.tellg();
    std::getline(m_sFileHandle, buf);
    if (!buf.empty()) m_vLineIndices.push_back(pos);
  }
  return true;
}

std::string CFileWordGenerator::GetLine() {
  if (m_vLineIndices.empty()) return "";
  if (m_sFileHandle.eof() && !m_sFileHandle.bad()) {
    //no error has occurred, but we've reached the end of file. So reset state as we're about to seek...
    m_sFileHandle.clear();
  }  
  int i = rand() % m_vLineIndices.size();
  m_sFileHandle.seekg(m_vLineIndices[i]);
  m_vLineIndices.erase(m_vLineIndices.begin()+i);
  //TODO: call DashIntf::Message, and return "", instead?
  if (!m_sFileHandle.good()) throw std::runtime_error("File I/O error reading "+m_sPath);
  string sRes;
  std::getline(m_sFileHandle, sRes);
  DASHER_ASSERT(!sRes.empty()); //shouldn't have stored index of empty string
  return sRes;
}


