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
#include "EventHandler.h"

#include <cstring>
#include <cstdlib>
#include <iostream>

using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

Dasher::CParamTables CSettingsStore::s_oParamTables;

// TODO: Don't propagate changes which don't affect anything.

CSettingsStore::CSettingsStore(Dasher::CEventHandler *pEventHandler):m_pEventHandler(pEventHandler) {
};

void CSettingsStore::LoadPersistent() {

  // Load each of the persistent parameters.  If we fail loading for the store, then 
  // we'll save the settings with the default value that comes from Parameters.h

  for(int i(0); i < NUM_OF_BPS; ++i) {
    bool bValue;
    if(s_oParamTables.BoolParamTable[i].persistent)
      if(LoadSetting(s_oParamTables.BoolParamTable[i].regName, &bValue))
        s_oParamTables.BoolParamTable[i].value = bValue;
      else
        SaveSetting(s_oParamTables.BoolParamTable[i].regName, s_oParamTables.BoolParamTable[i].value);            
  }

  for(int j(0); j < NUM_OF_LPS; ++j) {
    long lValue;
    if(s_oParamTables.LongParamTable[j].persistent)
      if(LoadSetting(s_oParamTables.LongParamTable[j].regName, &lValue)) 
        s_oParamTables.LongParamTable[j].value = lValue;
      else
        SaveSetting(s_oParamTables.LongParamTable[j].regName, s_oParamTables.LongParamTable[j].value);            
  }

  for(int k(0); k < NUM_OF_SPS; ++k) {
    std::string strValue;
    if(s_oParamTables.StringParamTable[k].persistent)
      if(LoadSetting(s_oParamTables.StringParamTable[k].regName, &strValue))
        s_oParamTables.StringParamTable[k].value = strValue;
      else
        SaveSetting(s_oParamTables.StringParamTable[k].regName, s_oParamTables.StringParamTable[k].value);            
  }
}


// Return 0 on success, an error string on failure.
const char * CSettingsStore::ClSet(const std::string &strKey, const std::string &strValue) {
  for(int i(0); i < NUM_OF_BPS; ++i) {
    if(strKey == s_oParamTables.BoolParamTable[i].regName) {
      if ((strValue == "0") || (strValue == _("true")))
	SetBoolParameter(s_oParamTables.BoolParamTable[i].key, false);
      else if((strValue == "1") || (strValue == _("false")))
	SetBoolParameter(s_oParamTables.BoolParamTable[i].key, true);
      else
        // Note to translators: This message will be output for a command line
        // with "--options foo=VAL" and foo is a boolean valued parameter, but
        // "VAL" is not true or false.
        return _("boolean value must be specified as 'true' or 'false'.");
      return 0;
    }
  }

  for(int i(0); i < NUM_OF_LPS; ++i) {
    if(strKey == s_oParamTables.LongParamTable[i].regName) {
      SetLongParameter(s_oParamTables.LongParamTable[i].key, atoi(strValue.c_str()));
      return 0;
    }
  }

  for(int i(0); i < NUM_OF_SPS; ++i) {
    if(strKey == s_oParamTables.StringParamTable[i].regName) {
      SetStringParameter(s_oParamTables.StringParamTable[i].key, strValue);
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
  DASHER_ASSERT(iParameter == s_oParamTables.BoolParamTable[iParameter - FIRST_BP].key);

  if(bValue == GetBoolParameter(iParameter))
    return;

  // Set the value
  s_oParamTables.BoolParamTable[iParameter - FIRST_BP].value = bValue;

  // Initiate events for changed parameter
  Dasher::CParameterNotificationEvent* oEvent = new Dasher::CParameterNotificationEvent(iParameter);

  m_pEventHandler->InsertEvent(oEvent);
  delete oEvent;

  // Write out to permanent storage
  if(s_oParamTables.BoolParamTable[iParameter - FIRST_BP].persistent)
    SaveSetting(s_oParamTables.BoolParamTable[iParameter - FIRST_BP].regName, bValue);
};

void CSettingsStore::SetLongParameter(int iParameter, long lValue) {

  // Check that the parameter is in fact in the right spot in the table
  DASHER_ASSERT(iParameter == s_oParamTables.LongParamTable[iParameter - FIRST_LP].key);

  if(lValue == GetLongParameter(iParameter))
    return;

  // Set the value
  s_oParamTables.LongParamTable[iParameter - FIRST_LP].value = lValue;

  // Initiate events for changed parameter
  Dasher::CParameterNotificationEvent oEvent(iParameter);
  m_pEventHandler->InsertEvent(&oEvent);

  // Write out to permanent storage
  if(s_oParamTables.LongParamTable[iParameter - FIRST_LP].persistent)
    SaveSetting(s_oParamTables.LongParamTable[iParameter - FIRST_LP].regName, lValue);
};

void CSettingsStore::SetStringParameter(int iParameter, const std::string sValue) {

  // Check that the parameter is in fact in the right spot in the table
  DASHER_ASSERT(iParameter == s_oParamTables.StringParamTable[iParameter - FIRST_SP].key);

  if(sValue == GetStringParameter(iParameter))
    return;

  // Set the value
  s_oParamTables.StringParamTable[iParameter - FIRST_SP].value = sValue;

  // Initiate events for changed parameter
  Dasher::CParameterNotificationEvent oEvent(iParameter);
  m_pEventHandler->InsertEvent(&oEvent);

  // Write out to permanent storage
  if(s_oParamTables.StringParamTable[iParameter - FIRST_SP].persistent)
    SaveSetting(s_oParamTables.StringParamTable[iParameter - FIRST_SP].regName, sValue);
};

bool CSettingsStore::GetBoolParameter(int iParameter) {
  // Check that the parameter is in fact in the right spot in the table
  DASHER_ASSERT(iParameter == s_oParamTables.BoolParamTable[iParameter - FIRST_BP].key);

  // Return the value
  return s_oParamTables.BoolParamTable[iParameter - FIRST_BP].value;
};

long CSettingsStore::GetLongParameter(int iParameter) {
  // Check that the parameter is in fact in the right spot in the table
  DASHER_ASSERT(iParameter == s_oParamTables.LongParamTable[iParameter - FIRST_LP].key);

  // Return the value
  return s_oParamTables.LongParamTable[iParameter - FIRST_LP].value;
};

std::string CSettingsStore::GetStringParameter(int iParameter) {
  // Check that the parameter is in fact in the right spot in the table
  DASHER_ASSERT(iParameter == s_oParamTables.StringParamTable[iParameter - FIRST_SP].key);

  // Return the value
  return s_oParamTables.StringParamTable[iParameter - FIRST_SP].value;
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

// Used to determine what data type a given parameter ID is.
ParameterType CSettingsStore::GetParameterType(int iParameter)
{
  if ((iParameter >= FIRST_BP) && (iParameter < FIRST_LP))
    return ParamBool;
  if ((iParameter >= FIRST_LP) && (iParameter < FIRST_SP))
    return ParamLong;
  if ((iParameter >= FIRST_SP) && (iParameter < END_OF_SPS))
    return ParamString;

  return ParamInvalid;
}

// Gets the string name for the given parameter
std::string CSettingsStore::GetParameterName(int iParameter)
{
  // Pull the registry name out of the correct table depending on the parameter type
  switch (GetParameterType(iParameter))
  {
  case (ParamBool):
    {
      DASHER_ASSERT(iParameter == s_oParamTables.BoolParamTable[iParameter - FIRST_BP].key);
      return s_oParamTables.BoolParamTable[iParameter - FIRST_BP].regName;
      break;
    }
  case (ParamLong):
    {
      DASHER_ASSERT(iParameter == s_oParamTables.LongParamTable[iParameter - FIRST_LP].key);
      return s_oParamTables.LongParamTable[iParameter - FIRST_LP].regName;
      break;
    }
  case (ParamString):
    {
      DASHER_ASSERT(iParameter == s_oParamTables.StringParamTable[iParameter - FIRST_SP].key);
      return s_oParamTables.StringParamTable[iParameter - FIRST_SP].regName;
      break;
    }
  case ParamInvalid:
    // TODO: Error handling?
    break;
  };


  return "";
}


////////////////////////////////////////////////////////////
//// DEPRECATED FUNCTIONS BELOW

bool CSettingsStore::GetBoolOption(const std::string &Key) {
  // Also make the call to the newer implementation
  for(int ii = 0; ii < NUM_OF_BPS; ii++) {
    if(s_oParamTables.BoolParamTable[ii].regName == Key) {
      return GetBoolParameter(s_oParamTables.BoolParamTable[ii].key);
    }
  }

  // This means the key passed in a string was not found in the new table
  DASHER_ASSERT(0);
  return false;
}

long CSettingsStore::GetLongOption(const std::string &Key) {
  for(int ii = 0; ii < NUM_OF_LPS; ii++) {
    if(s_oParamTables.LongParamTable[ii].regName == Key) {
      return GetLongParameter(s_oParamTables.LongParamTable[ii].key);
    }
  }
  // This means the key passed in a string was not found in the new table
  DASHER_ASSERT(0);
  return false;
}

std::string CSettingsStore::GetStringOption(const std::string &Key) {
  // Also make the call to the newer implementation
  for(int ii = 0; ii < NUM_OF_SPS; ii++) {
    if(s_oParamTables.StringParamTable[ii].regName == Key) {

      return GetStringParameter(s_oParamTables.StringParamTable[ii].key);
    }
  }

  // This means the key passed in a string was not found in the new table
  DASHER_ASSERT(0);
  return false;
}

void CSettingsStore::SetBoolOption(const std::string &Key, bool Value) {
  // Also make the call to the newer implementation
  for(int ii = 0; ii < NUM_OF_BPS; ii++) {
    if(s_oParamTables.BoolParamTable[ii].regName == Key) {
      SetBoolParameter(s_oParamTables.BoolParamTable[ii].key, Value);
      return;
    }
  }

  // This means the key passed in a string was not found in the new table
  DASHER_ASSERT(0);
}

void CSettingsStore::SetLongOption(const std::string &Key, long Value) {
  // Also make the call to the newer implementation
  for(int ii = 0; ii < NUM_OF_LPS; ii++) {
    if(s_oParamTables.LongParamTable[ii].regName == Key) {
      SetLongParameter(s_oParamTables.LongParamTable[ii].key, Value);
      return;
    }
  }

  // This means the key passed in a string was not found in the new table
  DASHER_ASSERT(0);
}

void CSettingsStore::SetStringOption(const std::string &Key, const std::string &Value) {
  // Also make the call to the newer implementation
  for(int ii = 0; ii < NUM_OF_SPS; ii++) {
    if(s_oParamTables.StringParamTable[ii].regName == Key) {
      SetStringParameter(s_oParamTables.StringParamTable[ii].key, Value);
      return;
    }
  }

  // This means the key passed in a string was not found in the new table
  DASHER_ASSERT(0);
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
