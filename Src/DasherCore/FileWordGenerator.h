#ifndef __FileWordGenerator_h__
#define __FileWordGenerator_h__

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
using namespace std;

#include "WordGeneratorBase.h"

namespace Dasher {
  
  
/**
 * This class implements the Word Generator interface. This means
 * that after construction, your primary method of retrieving words
 * will be calling GetNextWord(). For specifics on Word Generators,
 * see CWordGeneratorBase.
 * 
 * This class specifically reads words from a given file. Files ARE
 * kept open for the lifetime of this object and the size of the file
 * should not pose an issue. 
 * 
 * However, if you read in a file that has unreasonably long lines,
 * the behavior is undefined as you may cause the file to be read in all
 * at once. 
 */
class CFileWordGenerator : public CWordGeneratorBase, public AbstractParser {
public:
  CFileWordGenerator(CMessageDisplay *pMsgs, const CAlphInfo *pAlph, const CAlphabetMap *pAlphMap);

  ///Attempt to read from an arbitrary stream. Returns false, as we
  /// only support reading game mode sentences from files.
  bool Parse(const std::string &strDesc, std::istream &in, bool bUser);
  
  ///Attempt to open the specified file. Return true for success, false for failure
  bool ParseFile(const std::string &strFileName, bool bUser);
    
  virtual ~CFileWordGenerator() {
    m_sFileHandle.close();
  }

  /**
   * Return the next line from the file
   * @throw  Throws an exception if the file cannot be read.
   */
 virtual std::string GetLine();
  
  void setAcceptUser(bool bAcceptUser) {m_bAcceptUser = bAcceptUser;}

  bool HasLines() {return !m_vLineIndices.empty();}
  
private:

/* ---------------------------------------------------------------------
 * Member Variables
 * ---------------------------------------------------------------------
 */

  /**
   * The path to the file this generator reads from.
   */
  std::string m_sPath;
  
  /**
   * The input stream that acts as the handle to the underlying file.
   */
  ifstream m_sFileHandle;
  
  std::vector<streampos> m_vLineIndices;
  
  bool m_bAcceptUser;
};
}

#endif
