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
    
  private:
    CControlManager *m_pControlManager;
  };
  
}

#endif
