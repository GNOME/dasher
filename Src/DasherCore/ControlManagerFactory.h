#ifndef __controlmanagerfactory_h__
#define __controlmanagerfactory_h__

#include "ControlManager.h"
#include "NodeManagerFactory.h"

namespace Dasher {

  class CDasherModel;
  
  class CControlManagerFactory : public CNodeManagerFactory {
  public:
    CControlManagerFactory(CDasherModel *pModel);
    ~CControlManagerFactory();
    
    virtual CDasherNode *GetRoot();
    
  private:
    CControlManager *m_pControlManager;
  };
  
}

#endif
