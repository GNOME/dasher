#include "GtkDasherStore.h"
#include <iostream.h>

GtkDasherStore::GtkDasherStore()
{
  cout << "Warning - this version of Dasher has been built without support for storage" << endl << "of configuration settings. Changes made to the configuration will be lost" << endl << "when you exit Dasher." << endl;
}

bool GtkDasherStore::LoadSetting(const std::string& Key, bool* Value)
{
  //  cout << "Load Setting " << Key <<  endl;
  return( false );
}

bool GtkDasherStore::LoadSetting(const std::string& Key, long* Value)
{
  //  cout << "Load Setting " << Key << endl;
  return( false );
}

bool GtkDasherStore::LoadSetting(const std::string& Key, std::string* Value)
{
  //  cout << "Load Setting " << Key <<  endl;
  return( false );
}
	
void GtkDasherStore::SaveSetting(const std::string& Key, bool Value)
{
  //  cout << "Save Setting " << Key << ", " << Value << endl;
}

void GtkDasherStore::SaveSetting(const std::string& Key, long Value)
{
  //  cout << "Save Setting " << Key << ", " << Value << endl;
}

void GtkDasherStore::SaveSetting(const std::string& Key, const std::string& Value)
{
  //  cout << "Save Setting " << Key << ", " << Value << endl;
}
