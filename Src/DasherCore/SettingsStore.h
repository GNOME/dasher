// SettingsStore.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////



#ifndef __SettingsStore_h__
#define __SettingsStore_h__


#include "../Common/MSVC_Unannoy.h"
#include <string>
#include <map>


/*
	The public interface uses UTF-8 strings. All Keys should be
	in American English and encodable in ASCII. However,
	string Values may contain special characters where appropriate.
*/


class CSettingsStore
{
public:
	bool GetBoolOption(const std::string& Key);
	long GetLongOption(const std::string& Key);
	std::string& GetStringOption(const std::string& Key);
	
	void SetBoolOption(const std::string& Key, bool Value);
	void SetLongOption(const std::string& Key, long Value);
	void SetStringOption(const std::string& Key, const std::string& Value);
	
	void SetBoolDefault(const std::string& Key, bool Value);
	void SetLongDefault(const std::string& Key, long Value);
	void SetStringDefault(const std::string& Key, const std::string& Value);	
private:
	// Platform Specific settings file management

	// LoadSetting changes Value only if it succeeds in loading the setting,
	// in which case it also returns true. Failure is indicated by returning false.
	virtual bool LoadSetting(const std::string& Key, bool* Value);
	virtual bool LoadSetting(const std::string& Key, long* Value);
	virtual bool LoadSetting(const std::string& Key, std::string* Value);
	
	virtual void SaveSetting(const std::string& Key, bool Value);
	virtual void SaveSetting(const std::string& Key, long Value);
	virtual void SaveSetting(const std::string& Key, const std::string& Value);
	
	// Used to store settings in memory
	std::map<std::string, bool> BoolMap;
	std::map<std::string, long> LongMap;
	std::map<std::string, std::string> StringMap;
};


#endif /* #ifndef __SettingsStore_h__ */