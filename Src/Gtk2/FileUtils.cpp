#include <string>

#include "../Common/Globber.h"
#include "../DasherCore/AbstractXMLParser.h"
#include "../DasherCore/DasherInterfaceBase.h"
#include "FileUtils.h"

int FileUtils::GetFileSize(const std::string &strFileName) {
  struct stat sStatInfo;

  if(!stat(strFileName.c_str(), &sStatInfo))
    return sStatInfo.st_size;
  else
    return 0;
}

void FileUtils::ScanFiles(AbstractParser *parser, const std::string &strPattern) {
  //System files.
  // PROGDATA is provided by the makefile
  string path(PROGDATA "/");
  path += strPattern;

  std::string user_data_dir= getenv("HOME");
  user_data_dir += "/.dasher/";

  //User files.
  mkdir(user_data_dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  string user_path = user_data_dir;
  user_path += strPattern;

  const char *user[2], *sys[2];
  user[0] = user_path.c_str();
  sys[0] = path.c_str();
  user[1] = sys[1] = nullptr; //terminators

  globScan(parser, user, sys);
}


bool FileUtils::WriteUserDataFile(const std::string &filename, const std::string &strNewText, bool append) {
  if (strNewText.length() == 0)
    return true;

  std::string user_data_dir = getenv("HOME");
  user_data_dir += "/.dasher/";
  std::string strFilename(user_data_dir);
  strFilename = filename;
  int mode = O_CREAT | O_WRONLY;
  if (append)
  {
    mode = O_CREAT | O_WRONLY | O_APPEND;
  }
  int fd = open(strFilename.c_str(), mode, S_IRUSR | S_IWUSR);
  if (!fd) return false;
 
  int written = write(fd, strNewText.c_str(), strNewText.length());
  close(fd);
  return written == strNewText.length();

