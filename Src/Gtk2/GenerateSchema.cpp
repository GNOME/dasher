#include "../Common/Common.h"

#include "../DasherCore/Parameters.h"
#include "../Common/AppSettingsData.h"

#include <string>
#include <sstream>
#include <iostream>

enum{ TYPE_BOOL, TYPE_LONG, TYPE_STRING };

class CSchema {
public:
  CSchema(const std::string &strKeyName, int iType, 
	  const std::string &strDefault, const std::string &strShort,
	  const std::string &strLong);

  void Dump();
private:
  std::string m_strKeyName;
  std::string m_strType;
  std::string m_strDefault;
  std::string m_strShort;
  std::string m_strLong;
};

CSchema::CSchema(const std::string &strKeyName, int iType, 
		 const std::string &strDefault, const std::string &strShort,
		 const std::string &strLong) {
  m_strKeyName = strKeyName;
  
  switch(iType) {
  case TYPE_BOOL:
    m_strType="bool";
    break;
  case TYPE_LONG:
    m_strType="int";
    break;
  case TYPE_STRING:
    m_strType="string";
    break;
  }

  m_strDefault = strDefault;
  m_strShort = strShort;
  m_strLong = strLong;
}

void CSchema::Dump() {
  std::cout << "<schema>" << std::endl;
  std::cout << "<key>/schemas/apps/dasher4/" << m_strKeyName << "</key>" << std::endl;
  std::cout << "<applyto>/apps/dasher4/" << m_strKeyName << "</applyto>" << std::endl;
  std::cout << "<owner>dasher</owner>" << std::endl;
  std::cout << "<type>" << m_strType << "</type>" << std::endl;
  std::cout << "<default>" << m_strDefault << "</default>" << std::endl;
  std::cout << "<locale name=\"C\">" << std::endl;
  std::cout << "<short>" << m_strShort << "</short>" << std::endl;
  std::cout << "<long>" << m_strLong << "</long>" << std::endl;
  std::cout << "</locale>" << std::endl;
  std::cout << "</schema>" << std::endl;
  
}

int main(int argc, char **argv) {

  std::cout << "<gconfschemafile>" << std::endl;
  std::cout << "<schemalist>" << std::endl;
  std::cout << "Is this really main? -- Who knows?" << std::endl;

  for(int i(0); i < NUM_OF_BPS; ++i) {
    if(boolparamtable[i].persistent) {
      std::string strDefault;
      
      if(boolparamtable[i].defaultValue)
	strDefault = "TRUE";
      else
	strDefault = "FALSE";
      
      CSchema oSchema( boolparamtable[i].regName,
		       TYPE_BOOL,
		       strDefault,
		       "",
		       boolparamtable[i].humanReadable );
      
      oSchema.Dump();
    }
  }  

  for(int i(0); i < END_OF_APP_BPS - END_OF_SPS; ++i) {

    if(app_boolparamtable[i].persistent) {
      std::string strDefault;
      
      if(app_boolparamtable[i].bDefaultValue)
	strDefault = "TRUE";
      else
	strDefault = "FALSE";
      
      CSchema oSchema( app_boolparamtable[i].regName,
		       TYPE_BOOL,
		       strDefault,
		       "",
		       app_boolparamtable[i].humanReadable );
      
      oSchema.Dump();
    }
  } 

  for(int i(0); i < NUM_OF_LPS; ++i) {
    if(longparamtable[i].persistent) {

      std::stringstream ssDefault;
      
      ssDefault << longparamtable[i].defaultValue;

      
      CSchema oSchema( longparamtable[i].regName,
		       TYPE_LONG,
		       ssDefault.str(),
		       "",
		       longparamtable[i].humanReadable );
      
      oSchema.Dump();
    }
  }  
 
  for(int i(0); i < END_OF_APP_LPS - END_OF_APP_BPS; ++i) {
    if(app_longparamtable[i].persistent) {

      std::stringstream ssDefault;
      
      ssDefault << app_longparamtable[i].iDefaultValue;

      
      CSchema oSchema( app_longparamtable[i].regName,
		       TYPE_LONG,
		       ssDefault.str(),
		       "",
		       app_longparamtable[i].humanReadable );
      
      oSchema.Dump();
    }
  }  

  for(int i(0); i < NUM_OF_SPS; ++i) {
    if(stringparamtable[i].persistent) {
      CSchema oSchema( stringparamtable[i].regName,
		       TYPE_STRING,
		       stringparamtable[i].defaultValue,
		       "",
		       stringparamtable[i].humanReadable );
      
      oSchema.Dump();
    }
  } 
  
  for(int i(0); i < END_OF_APP_SPS - END_OF_APP_LPS; ++i) {
    if(app_stringparamtable[i].persistent) {
      CSchema oSchema( app_stringparamtable[i].regName,
		       TYPE_STRING,
		       app_stringparamtable[i].szDefaultValue,
		       "",
		       app_stringparamtable[i].humanReadable );
      
      oSchema.Dump();
    }
  } 
  
  std::cout << "</schemalist>" << std::endl;
  std::cout << "</gconfschemafile>" << std::endl;
}
// struct bp_table {
//   int key;
//   char *regName;
//   bool persistent;
//   bool defaultValue;
//   char *humanReadable;
// };
