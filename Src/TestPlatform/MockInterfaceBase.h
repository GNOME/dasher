#ifndef __MockInterfaceBase_h__
#define __MockInterfaceBase_h__

#include "../DasherCore/DasherInterfaceBase.h"

using namespace Dasher;

/**
 * A useless, but concrete implementation of CDasherInterfaceBase
 * used for unit testing purposes. Allows us to instantiate 
 * CDasherInterfaceBase without delving into platform specific code.
 */
class CMockInterfaceBase : public CDasherInterfaceBase {
  
  public:
  
    CMockInterfaceBase() : CDasherInterfaceBase() {};
    
    void CreateModules() {};
    
    void SetupPaths() {};
    
    void ScanAlphabetFiles(std::vector<std::string> &vFileList) {};
    
    void ScanColourFiles(std::vector<std::string> &vFileList) {};
    
    void SetupUI() {};
    
    void CreateSettingsStore() {};
    
    int GetFileSize(const std::string &strFileName) { return 0; };
    
    void StartTimer() {};
    
    void ShutdownTimer() {};
  
};

#endif
