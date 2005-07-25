#ifndef __nodemanagerfactory_h__
#define __nodemanagerfactory_h__


namespace Dasher {

  class CDasherNode;

class CNodeManagerFactory {
 public:
  virtual CDasherNode *GetRoot() = 0;
};

}
#endif
