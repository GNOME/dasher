#ifndef __controlmanagerfactory_h__
#define __controlmanagerfactory_h__

#include "LanguageModelling/LanguageModel.h"
#include "DasherNode.h"

class CNodeCreationManager;

namespace Dasher {
 class CControlManager;
}

namespace Dasher {
  class CDasherModel;

  /// \ingroup Model
  /// @{
  class CControlManagerFactory {
  public:
    CControlManagerFactory(CNodeCreationManager *pNCManager);
    ~CControlManagerFactory();
    
    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData);

    // Control mode stuff
    void RegisterNode( int iID, const std::string &strLabel, int iColour );
    void ConnectNode(int iChild, int iParent, int iAfter);
    void DisconnectNode(int iChild, int iParent);
    
  private:
    CControlManager *m_pControlManager;
  };
  /// @}
}

#endif
