#include "../Common/Common.h"

#include "../DasherCore/Parameters.h"
#include "../Common/AppSettingsData.h"

#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace Dasher::Settings;

enum EValType { TYPE_BOOL, TYPE_LONG, TYPE_STRING };
enum EOutput { GCONF_OUTPUT, GSETTINGS_OUTPUT, TEXT_OUTPUT };

class CSchema {
public:
  CSchema(const std::string &strKeyName, EValType iType, 
	  const std::string &strDefault, const std::string &strShort,
	  const std::string &strLong);

  void Dump(EOutput);
private:
  std::string m_strKeyName;
  EValType    m_iType;
  std::string m_strDefault;
  std::string m_strShort;
  std::string m_strLong;
};

struct lower_char
{
	void operator()(char &c) {c = tolower(c);}
};

std::string lower(const std::string& in)
{
	std::string out(in);
	for_each(out.begin(), out.end(), lower_char());
	return out;
}

CSchema::CSchema(const std::string &strKeyName, EValType iType, 
		 const std::string &strDefault, const std::string &strShort,
		 const std::string &strLong) {
  m_strKeyName = strKeyName;
  m_iType = iType;
  m_strDefault = strDefault;
  m_strShort = strShort;
  m_strLong = strLong;
}

void CSchema::Dump(EOutput output_type) {
  switch(output_type) {
  case GCONF_OUTPUT:
    std::cout << "<schema>" << std::endl;
    std::cout << "<key>/schemas/apps/dasher4/" << m_strKeyName << "</key>" << std::endl;
    std::cout << "<applyto>/apps/dasher4/" << m_strKeyName << "</applyto>" << std::endl;
    std::cout << "<owner>dasher</owner>" << std::endl;

    std::cout << "<type>";
    switch(m_iType) {
    case TYPE_BOOL:
      std::cout << "bool";
      break;
    case TYPE_LONG:
      std::cout << "int";
      break;
    case TYPE_STRING:
      std::cout << "string";
      break;
    }
    std::cout << "</type>" << std::endl;

    std::cout << "<default>" << m_strDefault << "</default>" << std::endl;
    std::cout << "<locale name=\"C\">" << std::endl;
    std::cout << "<short>" << m_strShort << "</short>" << std::endl;
    std::cout << "<long>" << m_strLong << "</long>" << std::endl;
    std::cout << "</locale>" << std::endl;
    std::cout << "</schema>" << std::endl;
    break;
  case GSETTINGS_OUTPUT:
    std::cout << "    <key name=\"" << m_strKeyName << "\" type=\"";
    switch(m_iType) {
    case TYPE_BOOL:
      std::cout << 'b';
      break;
    case TYPE_LONG:
      std::cout << 'i';
      break;
    case TYPE_STRING:
      std::cout << 's';
      break;
    }
    std::cout << "\">\n";

    std::cout << "      <default>" << (m_iType==TYPE_STRING?"'":"");
    if (m_iType==TYPE_BOOL)
      std::cout << lower(m_strDefault);
    else
      std::cout << m_strDefault;
    std::cout << (m_iType==TYPE_STRING?"'":"") << "</default>\n";

    if (!m_strShort.empty())
      std::cout << "      <summary>" << m_strShort << "</summary>\n";
    if (!m_strLong.empty())
      std::cout << "      <description>" << m_strLong << "</description>\n";
    std::cout << "    </key>\n";

    break;
  case TEXT_OUTPUT:
    std::cout << std::setw(30)
              << m_strKeyName << '\t';
    switch(m_iType) {
    case TYPE_BOOL:
      std::cout << "bool\t";
      break;
    case TYPE_LONG:
      std::cout << "int\t";
      break;
    case TYPE_STRING:
      std::cout << "string\t";
      break;
    }
    std::cout << m_strDefault << '\t'
              << m_strShort   << '\n';
    break;
  }
}

void usage(char *progname) {
	std::cerr << "usage: " << progname
		<< " [-cst]\n-c: GConf\n-s: GSettings\n-t: text\n";
}

int main(int argc, char **argv) {

	EOutput output;

	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
	if (argv[1][0] != '-' || argv[1][2] != '\0') {
		usage(argv[0]);
		return 1;
	}
	switch(argv[1][1]) {
		case 'c':
			output = GCONF_OUTPUT;
			break;
		case 's':
			output = GSETTINGS_OUTPUT;
			break;
		case 't':
			output = TEXT_OUTPUT;
			break;
		default:
			usage(argv[0]);
			return 1;
	}

  switch (output) {
  case GCONF_OUTPUT:
    std::cout << "<gconfschemafile>" << std::endl;
    std::cout << "<schemalist>" << std::endl;
    std::cout << "Is this really main? -- Who knows?" << std::endl;
    break;
  case GSETTINGS_OUTPUT:
    std::cout << "<schemalist>\n"
                 "  <schema id=\"org.gnome.Dasher\" path=\"/apps/dasher4/\">\n";
    break;
  }

  for(int i(0); i < NUM_OF_BPS; ++i) {
    std::string strDefault(boolparamtable[i].defaultValue ? "TRUE" : "FALSE");
      
    CSchema oSchema( boolparamtable[i].regName,
		       TYPE_BOOL,
		       strDefault,
		       "",
		       boolparamtable[i].humanReadable );
      
    oSchema.Dump(output);
  }  

  for(int i(0); i < END_OF_APP_BPS - END_OF_SPS; ++i) {

    if(app_boolparamtable[i].persistent == Persistence::PERSISTENT) {
      std::string strDefault;
      
      if(app_boolparamtable[i].defaultValue)
	strDefault = "TRUE";
      else
	strDefault = "FALSE";
      
      CSchema oSchema( app_boolparamtable[i].regName,
		       TYPE_BOOL,
		       strDefault,
		       "",
		       app_boolparamtable[i].humanReadable );
      
      oSchema.Dump(output);
    }
  } 

  for(int i(0); i < NUM_OF_LPS; ++i) {
    std::stringstream ssDefault;
      
    ssDefault << longparamtable[i].defaultValue;

      
    CSchema oSchema( longparamtable[i].regName,
		       TYPE_LONG,
		       ssDefault.str(),
		       "",
		       longparamtable[i].humanReadable );
      
    oSchema.Dump(output);
  }  
 
  for(int i(0); i < END_OF_APP_LPS - END_OF_APP_BPS; ++i) {
    if(app_longparamtable[i].persistent == Persistence::PERSISTENT) {

      std::stringstream ssDefault;
      
      ssDefault << app_longparamtable[i].defaultValue;

      
      CSchema oSchema( app_longparamtable[i].regName,
		       TYPE_LONG,
		       ssDefault.str(),
		       "",
		       app_longparamtable[i].humanReadable );
      
      oSchema.Dump(output);
    }
  }  

  for(int i(0); i < NUM_OF_SPS; ++i) {
    CSchema oSchema( stringparamtable[i].regName,
		       TYPE_STRING,
		       stringparamtable[i].defaultValue,
		       "",
		       stringparamtable[i].humanReadable );
      
    oSchema.Dump(output);
  } 
  
  for(int i(0); i < END_OF_APP_SPS - END_OF_APP_LPS; ++i) {
    if(app_stringparamtable[i].persistent == Persistence::PERSISTENT) {
      CSchema oSchema( app_stringparamtable[i].regName,
		       TYPE_STRING,
		       app_stringparamtable[i].defaultValue,
		       "",
		       app_stringparamtable[i].humanReadable );
      
      oSchema.Dump(output);
    }
  } 
  
  switch (output) {
  case GCONF_OUTPUT:
    std::cout << "</schemalist>" << std::endl;
    std::cout << "</gconfschemafile>" << std::endl;
    break;
  case GSETTINGS_OUTPUT:
    std::cout << "  </schema>\n</schemalist>" << std::endl;
  }

  return 0;
}
// struct bp_table {
//   int key;
//   char *regName;
//   bool persistent;
//   bool defaultValue;
//   char *humanReadable;
// };
