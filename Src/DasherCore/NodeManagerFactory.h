#ifndef __nodemanagerfactory_h__
#define __nodemanagerfactory_h__

namespace Dasher {
  class CDasherNode;
  /// \ingroup Model
  /// @{
  class CNodeManagerFactory {
  public:
    virtual CDasherNode *GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) = 0;
  };
  /// @}
}

#endif
