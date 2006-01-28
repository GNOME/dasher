
#include "../Common/Common.h"

#include "DashEdit.h"
#include "Event.h"
#include "EventHandler.h"

using namespace Dasher;

CDashEditbox::CDashEditbox():m_dirty(false), m_pDasherInterface(0) {}

CDashEditbox::~CDashEditbox() {}

void CDashEditbox::SetInterface(CDasherInterfaceBase * DasherInterface) {
  m_pDasherInterface = DasherInterface;
}

void CDashEditbox::HandleEvent(CEvent * pEvent) {
  if(pEvent->m_iEventType == 2) {
    Dasher::CEditEvent * pEvt(static_cast< Dasher::CEditEvent * >(pEvent));

    switch (pEvt->m_iEditType) {
    case 1:
      output(pEvt->m_sText);
      break;
    case 2:
      deletetext(pEvt->m_sText);
      break;
    }
  }
  else if(pEvent->m_iEventType == 3) {
    // FIXME - need to implement this
    /*
    Dasher::CEditContextEvent * pEvt(static_cast < Dasher::CEditContextEvent * >(pEvent));
    */
  }
}
