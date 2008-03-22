// SettingsStore.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __SettingsStore_h__
#define __SettingsStore_h__

#include <string>
#include <map>
#include "Parameters.h"

namespace Dasher {
  class CEventHandler;
  class CParameterNotificationEvent;
}

class Dasher::CEventHandler;
class Dasher::CParameterNotificationEvent;

// Types that are parameters can be
enum ParameterType
{
  ParamBool,
  ParamLong,
  ParamString,
  ParamInvalid
};

/// \ingroup Core
/// @{

/// \brief Abstract representation of persistant storage.
///
/// The public interface uses UTF-8 strings. All Keys should be
/// in American English and encodable in ASCII. However,
/// string Values may contain special characters where appropriate.
class CSettingsStore {
public:

  CSettingsStore(Dasher::CEventHandler * pEventHandler);

  virtual ~ CSettingsStore() {
  };

  // New functions for event driven interface

  void SetBoolParameter(int iParameter, bool bValue);
  void SetLongParameter(int iParameter, long lValue);
  void SetStringParameter(int iParameter, const std::string sValue);

  bool GetBoolParameter(int iParameter);
  long GetLongParameter(int iParameter);
  std::string GetStringParameter(int iParameter);

  void ResetParameter(int iParameter);

  ParameterType   GetParameterType(int iParameter);
  std::string     GetParameterName(int iParameter);

  void LoadPersistent();

  const char *ClSet(const std::string &strKey, const std::string &strValue);

  // --- Deprecated functions below
  // although string functionality might be good for
  // reading/writing from permanent storage (string based)

  bool GetBoolOption(const std::string & Key);
  long GetLongOption(const std::string & Key);
  std::string GetStringOption(const std::string & Key);

  void SetBoolOption(const std::string & Key, bool Value);
  void SetLongOption(const std::string & Key, long Value);
  void SetStringOption(const std::string & Key, const std::string & Value);

private:
  // Platform Specific settings file management

  // LoadSetting changes Value only if it succeeds in loading the setting,
  // in which case it also returns true. Failure is indicated by returning false.
  //! Load a setting with a boolean value
  //
  //! Load a setting with a boolean value. Return true if successful
  //! \param Key Name of the setting
  //! \param Value Value of the setting
  virtual bool LoadSetting(const std::string & Key, bool * Value);

  //! Load a setting with a long value
  //
  //! Load a setting with a long value. Return true if successful
  //! \param Key Name of the setting
  //! \param Value Value of the setting
  virtual bool LoadSetting(const std::string & Key, long *Value);

  //! Load a setting with a string value
  //
  //! Load a setting with a string value. Return true if successful
  //! \param Key Name of the setting
  //! \param Value Value of the setting, UTF8 encoded
  virtual bool LoadSetting(const std::string & Key, std::string * Value);

  //! Save a setting with a boolean value
  //
  //! \param Key Name of the setting
  //! \param Value Value of the setting
  virtual void SaveSetting(const std::string & Key, bool Value);

  //! Save a setting with a long value
  //
  //! \param Key Name of the setting
  //! \param Value Value of the setting
  virtual void SaveSetting(const std::string & Key, long Value);

  //! Save a setting with a string value
  //
  //! \param Key Name of the setting
  //! \param Value Value of the setting, UTF8 encoded
  virtual void SaveSetting(const std::string & Key, const std::string & Value);

protected:
  Dasher::CEventHandler * m_pEventHandler;

  // This is where the settings are, should only be one in existance
  // derived classes should share this reference

  static Dasher::CParamTables s_oParamTables;
};
/// @}

#endif /* #ifndef __SettingsStore_h__ */
