// DasherSettingsInterface.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __DasherSettingsInterface_h__
#define __DasherSettingsInterface_h__


#include "DasherTypes.h"
#include "SettingsStore.h"

namespace Dasher {
    class CDasherSettingsInterface;
    class CDasherInterfaceBase;
}

class Dasher::CDasherSettingsInterface
{
public:
    virtual void HandleParameterNotification( int iParameter );

    void SetInterface( Dasher::CDasherInterfaceBase *pInterface ) {
        m_pInterface = pInterface;
    };

	// New externally visible interface
    void SetBoolParameter( int iParameter, bool bValue );
    void SetLongParameter( int iParameter, long lValue );
    void SetStringParameter( int iParameter, const std::string &sValue );

    bool GetBoolParameter( int iParameter );
    long GetLongParameter( int iParameter );
    std::string GetStringParameter( int iParameter );

private:
  Dasher::CDasherInterfaceBase *m_pInterface;

};
#endif /* #ifndef __DasherSettingsInterface_h__ */




