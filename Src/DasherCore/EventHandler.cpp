#include "EventHandler.h"
#include "DasherComponent.h"
#include "DasherInterfaceBase.h"

#include <algorithm>

using namespace Dasher;

void CEventHandler::InsertEvent( CEvent *pEvent ) {

	m_pInterface->ExternalEventHandler( pEvent );
	
  for( std::vector< CDasherComponent * >::iterator iCurrent( m_vListeners.begin() ); iCurrent != m_vListeners.end(); ++iCurrent) {
    (*iCurrent)->HandleEvent( pEvent );
  }
}

void CEventHandler::RegisterListener( CDasherComponent *pListener ) {

  if( std::find( m_vListeners.begin(), m_vListeners.end(), pListener ) == m_vListeners.end() ) {
    m_vListeners.push_back( pListener );
  }
  else {
    // Can't add the same listener twice
  }
}

void CEventHandler::UnregisterListener( CDasherComponent *pListener ) {

  std::vector< CDasherComponent * >::iterator iFound;

  iFound = std::find( m_vListeners.begin(), m_vListeners.end(), pListener );

  if( iFound != m_vListeners.end() )
    m_vListeners.erase( iFound );
}
