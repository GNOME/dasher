// SettingsStore.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////




#include "SettingsStore.h"

using namespace std;

/* TODO: Consider using Template functions to make this neater. */


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
}


void CSettingsStore::SetLongOption(const string& Key, long Value)
{
	LongMap[Key] = Value;
	SaveSetting(Key, Value);
}


void CSettingsStore::SetStringOption(const string& Key, const string& Value)
{
	StringMap[Key] = Value;
	SaveSetting(Key, Value);
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
