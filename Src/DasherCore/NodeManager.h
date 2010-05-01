#ifndef __nodemanager_h__
#define __nodemanager_h__
namespace Dasher {
  
  /// A marker class for anything that can be returned by CDasherNode::mgr()
  ///  - as a void* return type can't be covariantly overridden :-(
  class CNodeManager {
  };
}
#endif
