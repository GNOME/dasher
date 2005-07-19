#ifndef __LIB_EXPT_H__
#define __LIB_EXPT_H__

/// A class for managing computational experiments
///
/// (c) 2005 P. J. Cowans

// TODO - implement a way of sharing data between related runs

#include <iostream>
#include <fstream>
#include <string>
#include <map>

class cExperiment {
public:
  cExperiment(const std::string & oPrefix) {

    std::string oParameterLogFilename(oPrefix + ".params");
    std::string oDataLogFilename(oPrefix + ".data");

    oParameterLog.open(oParameterLogFilename.c_str());
    oDataLog.open(oDataLogFilename.c_str());

  };

  ~cExperiment() {
    oParameterLog.close();
    oDataLog.close();
  }

  /// Set a parameter

  // TODO - can we do anything sensible with templates?

  inline void SetParameterInt(const std::string & sName, int iValue) {
    oParameterMapInt[sName] = iValue;
  }

  inline void SetParameterString(const std::string & sName, const std::string & sValue) {
    oParameterMapString[sName] = sValue;
  }

  /// Get a parameter

  inline int GetParameterInt(const std::string & sName) const {
    // TODO - handle case when parameter is not set (throw an exception)

    std::map < std::string, int >::const_iterator it(oParameterMapInt.find(sName));

    if(it == oParameterMapInt.end()) {
      std::cerr << "Error - asked for non-initialised parameter " << sName << std::endl;
      exit(1);
    }
    else {
      std::cerr << sName << " " << it->second << std::endl;
    }

    return it->second;
  }

  inline std::string GetParameterString(const std::string & sName)const {
    // TODO - handle case when parameter is not set (throw an exception)

    std::map < std::string, std::string >::const_iterator it(oParameterMapString.find(sName));

    if(it == oParameterMapString.end()) {
      std::cerr << "Error - asked for non-initialised parameter " << sName << std::endl;
      exit(1);
    }
    else {
      std::cerr << sName << " " << it->second << std::endl;
    }

    return it->second;
  }

  /// Dump a list of the parameters

  void DumpParameters() {
    for(std::map < std::string, int >::iterator it(oParameterMapInt.begin()); it != oParameterMapInt.end(); ++it)
      oParameterLog << it->first << ": " << it->second << std::endl;

    for(std::map < std::string, std::string >::iterator it(oParameterMapString.begin()); it != oParameterMapString.end(); ++it)
      oParameterLog << it->first << ": " << it->second << std::endl;
  }

  /// Records a string in the log file

  void RecordData(const std::string & oData) {
    oDataLog << oData;
  }

  /// Actually do the experiment

  double Run() {
    DumpParameters();
    return Execute();
  }

  /// This method needs to be implemented by the child class - the actual work of doing the experiment happens here.

  virtual double Execute() = 0;

private:
  std::map < std::string, int >oParameterMapInt;
  std::map < std::string, std::string > oParameterMapString;
  std::ofstream oParameterLog;
  std::ofstream oDataLog;
};

#endif
