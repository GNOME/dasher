#include "ControlManager.h"
#include "DasherModel.h"
#include "DasherNode.h"

#include <vector>

using namespace Dasher;

CControlManager::CControlManager( CDasherModel *pModel )
  : m_pModel(pModel) {
}

CDasherNode *CControlManager::GetRoot() {
  CDasherNode *pNewNode;

  pNewNode->m_pNodeManager = NULL;

  return pNewNode;
}

void CControlManager::PopulateChildren( CDasherNode *pNode ) {

}

void CControlManager::ClearNode( CDasherNode *pNode ) {
  // Should this be responsible for actually doing the deletion
}
