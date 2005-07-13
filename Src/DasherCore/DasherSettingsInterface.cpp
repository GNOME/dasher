// DasherSettingsInterface.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

#include "DasherSettingsInterface.h"
#include "DasherInterfaceBase.h"
#include "Parameters.h"

void Dasher::CDasherSettingsInterface::SetBoolParameter( int iParameter, bool bValue ) {
	m_pInterface->SetBoolParameter( iParameter, bValue );
};

void Dasher::CDasherSettingsInterface::SetLongParameter( int iParameter, long lValue ) {
	m_pInterface->SetLongParameter( iParameter, lValue );
};

void Dasher::CDasherSettingsInterface::SetStringParameter( int iParameter, const std::string &sValue ) {
	m_pInterface->SetStringParameter( iParameter, sValue );
};

bool Dasher::CDasherSettingsInterface::GetBoolParameter( int iParameter ) {
    return m_pInterface->GetBoolParameter( iParameter );
}

long Dasher::CDasherSettingsInterface::GetLongParameter( int iParameter ) {
    return m_pInterface->GetLongParameter( iParameter );
}

std::string Dasher::CDasherSettingsInterface::GetStringParameter( int iParameter ) {
    return m_pInterface->GetStringParameter( iParameter );
}

void Dasher::CDasherSettingsInterface::HandleParameterNotification( int iParameter) {

}
