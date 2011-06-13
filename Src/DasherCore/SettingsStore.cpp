// SettingsStore.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

#include "SettingsStore.h"
#include "Event.h"
#include "Observable.h"

#include <cstring>
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace Dasher;
using namespace Dasher::Settings;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

static CSettingsStore *s_pSettingsStore = NULL;

CSettingsStore::CSettingsStore() {
}

void CSettingsStore::LoadPersistent() {

  // Load each of the persistent parameters.  If we fail loading for the store, then 
  // we'll save the settings with the default value that comes from Parameters.h

  for(int i(0); i < NUM_OF_BPS; ++i) {
    boolParamValues[i] = boolparamtable[i].defaultValue;
    if(boolparamtable[i].persistent) {
      bool bValue;
      if(LoadSetting(boolparamtable[i].regName, &bValue))
        boolParamValues[i] = bValue;
      else
        SaveSetting(boolparamtable[i].regName, boolParamValues[i]);
    }
  }

  for(int j(0); j < NUM_OF_LPS; ++j) {
    longParamValues[j] = longparamtable[j].defaultValue;
    if(longparamtable[j].persistent) {
      long lValue;
      if(LoadSetting(longparamtable[j].regName, &lValue)) 
        longParamValues[j] = lValue;
      else
        SaveSetting(longparamtable[j].regName, longParamValues[j]);            
    }
  }

  for(int k(0); k < NUM_OF_SPS; ++k) {
    stringParamValues[k] = stringparamtable[k].defaultValue;
    if(stringparamtable[k].persistent) {
      std::string strValue;
      if(LoadSetting(stringparamtable[k].regName, &strValue))
        stringParamValues[k] = strValue;
      else
        SaveSetting(stringparamtable[k].regName, stringParamValues[k]);            
    }
  }
}


// Return 0 on success, an error string on failure.
const char * CSettingsStore::ClSet(const std::string &strKey, const std::string &strValue) {
  for(int i(0); i < NUM_OF_BPS; ++i) {
    if(strKey == boolparamtable[i].regName) {
      if ((strValue == "0") || (strValue == _("true")))
	SetBoolParameter(boolparamtable[i].key, false);
      else if((strValue == "1") || (strValue == _("false")))
	SetBoolParameter(boolparamtable[i].key, true);
      else
        // Note to translators: This message will be output for a command line
        // with "--options foo=VAL" and foo is a boolean valued parameter, but
        // "VAL" is not true or false.
        return _("boolean value must be specified as 'true' or 'false'.");
      return 0;
    }
  }

  for(int i(0); i < NUM_OF_LPS; ++i) {
    if(strKey == longparamtable[i].regName) {
      SetLongParameter(longparamtable[i].key, atoi(strValue.c_str()));
      return 0;
    }
  }

  for(int i(0); i < NUM_OF_SPS; ++i) {
    if(strKey == stringparamtable[i].regName) {
      SetStringParameter(stringparamtable[i].key, strValue);
      return 0;
    }
  }
  // Note to translators: This is output when command line "--options" doesn't
  // specify a known option.
  return _("unknown option, use \"--help-options\" for more information.");
}


/* TODO: Consider using Template functions to make this neater. */

void CSettingsStore::SetBoolParameter(int iParameter, bool bValue) {

  // Check that the parameter is in fact in the right spot in the table
  DASHER_ASSERT(iParameter == boolparamtable[iParameter - FIRST_BP].key);

  if(bValue == GetBoolParameter(iParameter))
    return;

  // Set the value
  boolParamValues[iParameter - FIRST_BP] = bValue;

  // Initiate events for changed parameter
  DispatchEvent(iParameter);

  // Write out to permanent storage
  if(boolparamtable[iParameter - FIRST_BP].persistent)
    SaveSetting(boolparamtable[iParameter - FIRST_BP].regName, bValue);
}

void CSettingsStore::SetLongParameter(int iParameter, long lValue) {

  // Check that the parameter is in fact in the right spot in the table
  DASHER_ASSERT(iParameter == longparamtable[iParameter - FIRST_LP].key);

  if(lValue == GetLongParameter(iParameter))
    return;

  // Set the value
  longParamValues[iParameter - FIRST_LP] = lValue;

  // Initiate events for changed parameter
  DispatchEvent(iParameter);

  // Write out to permanent storage
  if(longparamtable[iParameter - FIRST_LP].persistent)
    SaveSetting(longparamtable[iParameter - FIRST_LP].regName, lValue);
}

void CSettingsStore::SetStringParameter(int iParameter, const std::string sValue) {

  // Check that the parameter is in fact in the right spot in the table
  DASHER_ASSERT(iParameter == stringparamtable[iParameter - FIRST_SP].key);

  if(sValue == GetStringParameter(iParameter))
    return;

  // Set the value
  stringParamValues[iParameter - FIRST_SP] = sValue;

  // Initiate events for changed parameter
  DispatchEvent(iParameter);

  // Write out to permanent storage
  if(stringparamtable[iParameter - FIRST_SP].persistent)
    SaveSetting(stringparamtable[iParameter - FIRST_SP].regName, sValue);
}

bool CSettingsStore::GetBoolParameter(int iParameter) const {
  // Check that the parameter is in fact in the right spot in the table
  DASHER_ASSERT(iParameter == boolparamtable[iParameter - FIRST_BP].key);

  // Return the value
  return boolParamValues[iParameter - FIRST_BP];
}

long CSettingsStore::GetLongParameter(int iParameter) const {
  // Check that the parameter is in fact in the right spot in the table
  DASHER_ASSERT(iParameter == longparamtable[iParameter - FIRST_LP].key);

  // Return the value
  return longParamValues[iParameter - FIRST_LP];
}

const std::string &CSettingsStore::GetStringParameter(int iParameter) const {
  // Check that the parameter is in fact in the right spot in the table
  DASHER_ASSERT(iParameter == stringparamtable[iParameter - FIRST_SP].key);

  // Return the value
  return stringParamValues[iParameter - FIRST_SP];
}

void CSettingsStore::ResetParameter(int iParameter) {
  switch(GetParameterType(iParameter)) {
  case ParamBool:
    SetBoolParameter(iParameter, boolparamtable[iParameter-FIRST_BP].defaultValue);
    break;
  case ParamLong:
    SetLongParameter(iParameter, longparamtable[iParameter-FIRST_LP].defaultValue);
    break;
  case ParamString:
    SetStringParameter(iParameter, stringparamtable[iParameter-FIRST_SP].defaultValue);
    break;
  case ParamInvalid:
    // TODO: Error handling?
    break;
  }
}

/* Private functions -- Settings are not saved between sessions unless these
functions are over-ridden.
--------------------------------------------------------------------------*/

bool CSettingsStore::LoadSetting(const std::string &Key, bool *Value) {
  return false;
}

bool CSettingsStore::LoadSetting(const std::string &Key, long *Value) {
  return false;
}

bool CSettingsStore::LoadSetting(const std::string &Key, std::string *Value) {
  return false;
}

void CSettingsStore::SaveSetting(const std::string &Key, bool Value) {
}

void CSettingsStore::SaveSetting(const std::string &Key, long Value) {
}

void CSettingsStore::SaveSetting(const std::string &Key, const std::string &Value) {
}

/* SettingsUser and SettingsObserver definitions... */

CSettingsUser::CSettingsUser(CSettingsStore *pSettingsStore) {
  //ATM, we only allow one settings store, total...
  DASHER_ASSERT(s_pSettingsStore==NULL);
  //but in future, remove that if we're allowing multiple settings stores to exist
  // concurrently.
  s_pSettingsStore = pSettingsStore;
}

CSettingsUser::CSettingsUser(CSettingsUser *pCreateFrom) {
  //No need to do anything atm; but in future, copy CSettingsStore pointer
  // from argument.
  DASHER_ASSERT(pCreateFrom);
}

CSettingsUser::~CSettingsUser() {
}

bool CSettingsUser::GetBoolParameter(int iParameter) const {return s_pSettingsStore->GetBoolParameter(iParameter);}
long CSettingsUser::GetLongParameter(int iParameter) const {return s_pSettingsStore->GetLongParameter(iParameter);}
const std::string &CSettingsUser::GetStringParameter(int iParameter) const {return s_pSettingsStore->GetStringParameter(iParameter);}
void CSettingsUser::SetBoolParameter(int iParameter, bool bValue) {s_pSettingsStore->SetBoolParameter(iParameter, bValue);}
void CSettingsUser::SetLongParameter(int iParameter, long lValue) {s_pSettingsStore->SetLongParameter(iParameter, lValue);}
void CSettingsUser::SetStringParameter(int iParameter, const std::string &strValue) {s_pSettingsStore->SetStringParameter(iParameter, strValue);}

CSettingsObserver::CSettingsObserver(CSettingsUser *pCreateFrom) {
  DASHER_ASSERT(pCreateFrom);
  s_pSettingsStore->Register(this);
}

CSettingsObserver::~CSettingsObserver() {
  s_pSettingsStore->Unregister(this);
}

CSettingsUserObserver::CSettingsUserObserver(CSettingsUser *pCreateFrom)
: CSettingsUser(pCreateFrom), CSettingsObserver(pCreateFrom) {
}
