// WinOptions.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __WinOptions_h__
#define __WinOptions_h__

#include "../../Common/NoClones.h"
#include <string>
#include <map>

#include "../WinHelper.h"

// Look for documenting comments here:
#include "../../DasherCore/SettingsStore.h"


class CWinOptions : public CSettingsStore, private NoClones
{
public:
	CWinOptions(const std::string& Group, const std::string& Product);
	~CWinOptions();

	bool LoadSetting(const std::string& Key, LPWINDOWPLACEMENT pwp);
	void SaveSetting(const std::string& Key, const LPWINDOWPLACEMENT pwp);

private:	
	// Platform Specific settings file management
	bool LoadSetting(const std::string& Key, bool* Value);
	bool LoadSetting(const std::string& Key, long* Value);
	bool LoadSetting(const std::string& Key, std::string* Value);
	bool LoadSettingT(const std::string& Key, Tstring* Value);
	
	void SaveSetting(const std::string& Key, bool Value);
	void SaveSetting(const std::string& Key, long Value);
	void SaveSetting(const std::string& Key, const std::string& Value);
	
	void SaveSettingT(const std::string& Key, const Tstring& TValue);

	// Used to store settings in memory
	std::map<std::string, bool> BoolMap;
	std::map<std::string, long> LongMap;
	std::map<std::string, std::string> StringMap;
	
	// Platform Specific helpers
	HKEY ProductKey;
	int GetOrCreate(HKEY hKey, LPCTSTR lpSubKey, REGSAM samDesired, HKEY* lpNewKey);
	// CARE! Users of GetlpByte must call delete[] on *Data after use.
	bool GetlpByte(const Tstring& key, BYTE** Data) const;
};


#endif /* #ifndef __WinOptions_h__ */