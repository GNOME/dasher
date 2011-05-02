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
class CFileWordGenerator : public CWordGeneratorBase {
public:
  CFileWordGenerator(const CAlphInfo *pAlph, const CAlphabetMap *pAlphMap);

  ///Attempt to open the specified file. Return true for success, false for failure
  bool open(const std::string &sPath);
    
  virtual ~CFileWordGenerator() {
    m_sFileHandle.close();
  }

  /**
   * Return the next line from the file
   * @throw  Throws an exception if the file cannot be read.
   */
 virtual std::string GetLine();


  /**
   * File path getter
   * @return The path to the file this generator reads from.
   */
  std::string GetPath();

  /**
   * File name getter. Returns the file name and extension, without
   * any slashes.
   * @return The actual name of the file being read from
   */
  std::string GetFilename();
  

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
};
}

#endif
