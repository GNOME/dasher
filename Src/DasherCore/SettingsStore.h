// SettingsStore.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////



#ifndef __SettingsStore_h__
#define __SettingsStore_h__


#include <string>
#include <map>

#include "EventHandler.h"
#include "Parameters.h"

/*
	The public interface uses UTF-8 strings. All Keys should be
	in American English and encodable in ASCII. However,
	string Values may contain special characters where appropriate.
*/


class CSettingsStore
{
public:

	CSettingsStore( Dasher::CEventHandler *pEventHandler ) : m_pEventHandler( pEventHandler ) {};
	virtual ~CSettingsStore() {};
	
	// New functions for event driven interface

	void SetBoolParameter( int iParameter, bool bValue );
	void SetLongParameter( int iParameter, long lValue );
	void SetStringParameter( int iParameter, const std::string &sValue );

	bool GetBoolParameter( int iParameter );

	// ---

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
        //! Load a setting with a boolean value
        //
        //! Load a setting with a boolean value. Return true if successful
        //! \param Key Name of the setting
	//! \param Value Value of the setting
	virtual bool LoadSetting(const std::string& Key, bool* Value);

        //! Load a setting with a long value
        //
        //! Load a setting with a long value. Return true if successful
        //! \param Key Name of the setting
	//! \param Value Value of the setting
	virtual bool LoadSetting(const std::string& Key, long* Value);

        //! Load a setting with a string value
        //
        //! Load a setting with a string value. Return true if successful
        //! \param Key Name of the setting
	//! \param Value Value of the setting, UTF8 encoded
	virtual bool LoadSetting(const std::string& Key, std::string* Value);
	
	//! Save a setting with a boolean value
	//
	//! \param Key Name of the setting
	//! \param Value Value of the setting
	virtual void SaveSetting(const std::string& Key, bool Value);

	//! Save a setting with a long value
	//
	//! \param Key Name of the setting
	//! \param Value Value of the setting
	virtual void SaveSetting(const std::string& Key, long Value);

	//! Save a setting with a string value
	//
	//! \param Key Name of the setting
	//! \param Value Value of the setting, UTF8 encoded
	virtual void SaveSetting(const std::string& Key, const std::string& Value);
	
	// Used to store settings in memory
	std::map<std::string, bool> BoolMap;
	std::map<std::string, long> LongMap;
	std::map<std::string, std::string> StringMap;

	std::map<int, bool> m_oBoolParameterMap;

protected:
	Dasher::CEventHandler *m_pEventHandler;
		
};


#endif /* #ifndef __SettingsStore_h__ */
