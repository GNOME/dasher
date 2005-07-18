// SettingsStore.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////


#include "../Common/Common.h"

#include "SettingsStore.h"
#include "DasherSettingsInterface.h"
#include "Event.h"

#include <iostream>

using namespace std;

Dasher::CParamTables CSettingsStore::s_oParamTables;

CSettingsStore::CSettingsStore( Dasher::CEventHandler *pEventHandler ) : m_pEventHandler( pEventHandler ) {};

void CSettingsStore::LoadPersistant() {

  for( int i(0); i < NUM_OF_BPS; ++i ) {
    bool bValue;
    if( LoadSetting( s_oParamTables.BoolParamTable[i].regName, &bValue ) )
      s_oParamTables.BoolParamTable[i].value = bValue;
  }

  for( int i(0); i < NUM_OF_LPS; ++i ) {
    long lValue;
    if( LoadSetting( s_oParamTables.LongParamTable[i].regName, &lValue ) )
      s_oParamTables.LongParamTable[i].value = lValue;
  }

  for( int i(0); i < NUM_OF_SPS; ++i ) {
    std::string strValue;
    if( LoadSetting( s_oParamTables.StringParamTable[i].regName, &strValue ) )
      s_oParamTables.StringParamTable[i].value = strValue;
  }
}

/* TODO: Consider using Template functions to make this neater. */

void CSettingsStore::SetBoolParameter( int iParameter, bool bValue ) {


    // Check that the parameter is in fact in the right spot in the table
    DASHER_ASSERT(iParameter == s_oParamTables.BoolParamTable[iParameter-FIRST_BP].key);

    // Set the value
    s_oParamTables.BoolParamTable[iParameter-FIRST_BP].value = bValue;

    // Initiate events for changed parameter
	Dasher::CParameterNotificationEvent oEvent( iParameter );
	m_pEventHandler->InsertEvent( &oEvent );

    // Write out to permanent storage
    SaveSetting(s_oParamTables.BoolParamTable[iParameter-FIRST_BP].regName, bValue);
};

void CSettingsStore::SetLongParameter( int iParameter, long lValue ) {
	
    // Check that the parameter is in fact in the right spot in the table
    DASHER_ASSERT(iParameter == s_oParamTables.LongParamTable[iParameter-FIRST_LP].key);

    std::cout << "Setting long parameter: " << s_oParamTables.LongParamTable[iParameter-FIRST_LP].regName << " " << lValue << std::endl;

    // Set the value
    s_oParamTables.LongParamTable[iParameter-FIRST_LP].value = lValue;

    // Initiate events for changed parameter
	Dasher::CParameterNotificationEvent oEvent( iParameter );
	m_pEventHandler->InsertEvent( &oEvent );

    // Write out to permanent storage
    SaveSetting(s_oParamTables.LongParamTable[iParameter-FIRST_LP].regName, lValue);
};

void CSettingsStore::SetStringParameter( int iParameter, const std::string sValue ) {

    // Check that the parameter is in fact in the right spot in the table
    DASHER_ASSERT(iParameter == s_oParamTables.StringParamTable[iParameter-FIRST_SP].key);

    // Set the value
    s_oParamTables.StringParamTable[iParameter-FIRST_SP].value = sValue;

    // Initiate events for changed parameter
	Dasher::CParameterNotificationEvent oEvent( iParameter );
	m_pEventHandler->InsertEvent( &oEvent );

    // Write out to permanent storage
    SaveSetting(s_oParamTables.StringParamTable[iParameter-FIRST_SP].regName, sValue);
};

bool CSettingsStore::GetBoolParameter( int iParameter ) {
    // Check that the parameter is in fact in the right spot in the table
    DASHER_ASSERT(iParameter == s_oParamTables.BoolParamTable[iParameter-FIRST_BP].key);

    // Return the value
    return s_oParamTables.BoolParamTable[iParameter-FIRST_BP].value;
};

long CSettingsStore::GetLongParameter( int iParameter ) {
    // Check that the parameter is in fact in the right spot in the table
    DASHER_ASSERT(iParameter == s_oParamTables.LongParamTable[iParameter-FIRST_LP].key);

    // Return the value
    return s_oParamTables.LongParamTable[iParameter-FIRST_LP].value;
};

std::string CSettingsStore::GetStringParameter( int iParameter ) {
    // Check that the parameter is in fact in the right spot in the table

  	DASHER_ASSERT(iParameter == s_oParamTables.StringParamTable[iParameter-FIRST_SP].key);

    // Return the value
    return s_oParamTables.StringParamTable[iParameter-FIRST_SP].value;
}


////////////////////////////////////////////////////////////
//// DEPRECATED FUNCTIONS BELOW

bool CSettingsStore::GetBoolOption(const string& Key)
{
	if (BoolMap.find(Key)==BoolMap.end()) {
		bool Value = false;
		LoadSetting(Key, &Value);
		BoolMap[Key] = Value;
	}
	
	return BoolMap[Key];
}


long CSettingsStore::GetLongOption(const string& Key)
{
	if (LongMap.find(Key)==LongMap.end()) {
		long Value = 0l;
		LoadSetting(Key, &Value);
		LongMap[Key] = Value;
	}
	
	return LongMap[Key];
}


string& CSettingsStore::GetStringOption(const string& Key)
{
	if (StringMap.find(Key)==StringMap.end()) {
		string Value = "";
		LoadSetting(Key, &Value);
		StringMap[Key] = Value;
	}
	
	return StringMap[Key];
}


void CSettingsStore::SetBoolOption(const string& Key, bool Value)
{
	BoolMap[Key] = Value;
	SaveSetting(Key, Value);

    // Also make the call to the newer implementation
    for(int ii = 0; ii<NUM_OF_BPS; ii++)
    {
        if(s_oParamTables.BoolParamTable[ii].regName == Key)
        {
            SetBoolParameter(s_oParamTables.BoolParamTable[ii].key, Value);
            return;
        }
    }

    // This means the key passed in a string was not found in the new table
    DASHER_ASSERT(0);
}


void CSettingsStore::SetLongOption(const string& Key, long Value)
{
	LongMap[Key] = Value;
	SaveSetting(Key, Value);

    // Also make the call to the newer implementation
    for(int ii = 0; ii<NUM_OF_LPS; ii++)
    {
        if(s_oParamTables.LongParamTable[ii].regName == Key)
        {
            SetLongParameter(s_oParamTables.LongParamTable[ii].key, Value);
            return;
        }
    }

    // This means the key passed in a string was not found in the new table
    DASHER_ASSERT(0);
}


void CSettingsStore::SetStringOption(const string& Key, const string& Value)
{
	StringMap[Key] = Value;
	SaveSetting(Key, Value);

    // Also make the call to the newer implementation
    for(int ii = 0; ii<NUM_OF_SPS; ii++)
    {
        if(s_oParamTables.StringParamTable[ii].regName == Key)
        {
            SetStringParameter(s_oParamTables.StringParamTable[ii].key, Value);
            return;
        }
    }

    // This means the key passed in a string was not found in the new table
    DASHER_ASSERT(0);
}


void CSettingsStore::SetBoolDefault(const string& Key, bool Value)
{
	bool TmpValue;
	if ( (BoolMap.find(Key)==BoolMap.end()) && (!LoadSetting(Key, &TmpValue)) )
		SetBoolOption(Key, Value);
}


void CSettingsStore::SetLongDefault(const string& Key, long Value)
{
	long TmpValue;
	if ( (LongMap.find(Key)==LongMap.end()) && (!LoadSetting(Key, &TmpValue)) )
		SetLongOption(Key, Value);
}


void CSettingsStore::SetStringDefault(const string& Key, const string& Value)
{
	string TmpValue;
	if ( (StringMap.find(Key)==StringMap.end()) && (!LoadSetting(Key, &TmpValue)) )
		SetStringOption(Key, Value);
}


/* Private functions -- Settings are not saved between sessions unless these
functions are over-ridden.
--------------------------------------------------------------------------*/


bool CSettingsStore::LoadSetting(const string& Key, bool* Value)
{
	return false;
}


bool CSettingsStore::LoadSetting(const string& Key, long* Value)
{
	return false;
}


bool CSettingsStore::LoadSetting(const string& Key, string* Value)
{
	return false;
}


void CSettingsStore::SaveSetting(const string& Key, bool Value)
{
}


void CSettingsStore::SaveSetting(const string& Key, long Value)
{
}


void CSettingsStore::SaveSetting(const string& Key, const string& Value)
{
}
