#ifndef __MockFileWordGenerator_h__
#define __MockFileWordGenerator_h__

#include "../DasherCore/WordGeneratorBase.h"
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
using namespace std;

// MOCK OBJECT FOR TESTING
  
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
class MockFileWordGenerator : public CWordGeneratorBase {
public:
  MockFileWordGenerator(std::string sPath, int iRegen) 
    : m_sPath(sPath),
      CWordGeneratorBase(iRegen) {
    Generate();
  }
    
  MockFileWordGenerator(std::string sPath)
    : m_sPath(sPath),
      CWordGeneratorBase() {
    Generate();
  }
      
  virtual ~MockFileWordGenerator() {
    m_sFileHandle.close();
  }

  /**
   * Returns the next word that was read
   * @return The next word in the word generator model if there is one.
   *          Otherwise, returns an empty string if there are no more
   *          words.
   * @throw  Throws an exception if the file cannot be read.
   */
  virtual std::string GetNextWord();


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
  
  
  std::string GetWholeBuffer() { return m_sGeneratedString; }
  std::ifstream& GetFileHandle() { return m_sFileHandle; }
  size_t GetStringPosition() { return m_uiPos; }
  

private:
/* ---------------------------------------------------------------------
 * Private Methods
 * ---------------------------------------------------------------------
 */
  
  /**
   * The generator function. Reads words from the specified file.
   * @return True if generating the new buffer was successful,
   *          false if we've reached the end of the file.
   * @throw Throws an exception if the file cannot be read.
   */ 
  bool Generate();


/* ---------------------------------------------------------------------
 * Member Variables
 * ---------------------------------------------------------------------
 */

  /**
   * The string that has been generated.
   */
  std::string m_sGeneratedString;

  /**
   * The path to the file this generator reads from.
   */
  std::string m_sPath;

  /**
   * The position we're currently reading from in the string.
   */
  size_t m_uiPos;
  
  /**
   * The input stream that acts as the handle to the underlying file.
   */
  ifstream m_sFileHandle;
};


#endif
