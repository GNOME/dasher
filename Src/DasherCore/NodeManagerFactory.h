#ifndef __nodemanagerfactory_h__
#define __nodemanagerfactory_h__


namespace Dasher {

  class CDasherNode;

class CNodeManagerFactory {
 public:
  virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper) = 0;
};

}
#endif
