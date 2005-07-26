#ifndef __controlmanagerfactory_h__
#define __controlmanagerfactory_h__

#include "ControlManager.h"
#include "NodeManagerFactory.h"
#include "LanguageModelling/LanguageModel.h"

namespace Dasher {

  class CDasherModel;
  
  class CControlManagerFactory : public CNodeManagerFactory {
  public:
    CControlManagerFactory(CDasherModel *pModel, CLanguageModel *pLanguageModel);
    ~CControlManagerFactory();
    
    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper);

  
    // Control mode stuff
    
    void RegisterNode( int iID, const std::string &strLabel, int iColour ) {
      m_pControlManager->RegisterNode(iID, strLabel, iColour);
    }
    
    void ConnectNode(int iChild, int iParent, int iAfter) {
      m_pControlManager->ConnectNode(iChild, iParent, iAfter);
    }
    
  private:
    CControlManager *m_pControlManager;
  };
  
}

#endif
