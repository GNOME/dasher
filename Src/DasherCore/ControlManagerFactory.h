#ifndef __controlmanagerfactory_h__
#define __controlmanagerfactory_h__

#include "NodeManagerFactory.h"
#include "LanguageModelling/LanguageModel.h"

namespace Dasher {
 class CControlManager;
}

namespace Dasher {

  class CDasherModel;
  
  class CControlManagerFactory : public CNodeManagerFactory {
  public:
    CControlManagerFactory(CDasherModel *pModel, CLanguageModel *pLanguageModel);
    ~CControlManagerFactory();
    
    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper);

    // Control mode stuff
    void RegisterNode( int iID, const std::string &strLabel, int iColour );
    void ConnectNode(int iChild, int iParent, int iAfter);
    
  private:
    CControlManager *m_pControlManager;
  };
  
}

#endif
