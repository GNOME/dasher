#ifndef DASHER_FILEUTILS_H
#define DASHER_FILEUTILS_H

#include <string>
#include "../DasherCore/DasherInterfaceBase.h"

class FileUtils : public CFileUtils {
public:
  ~FileUtils() override = default;
  int GetFileSize(const std::string &strFileName) override;
  void ScanFiles(AbstractParser *parser, const std::string &strPattern) override;
  bool WriteUserDataFile(const std::string &filename, const std::string &strNewText, bool append) override;
};

#endif //DASHER_FILEUTILS_H
