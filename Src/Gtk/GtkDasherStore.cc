#include "GtkDasherStore.h"
#include <iostream.h>

GtkDasherStore::GtkDasherStore()
{
}

bool GtkDasherStore::LoadSetting(const std::string& Key, bool* Value)
{
  cout << "Load Setting " << Key <<  endl;
  return( false );
}

bool GtkDasherStore::LoadSetting(const std::string& Key, long* Value)
{
  cout << "Load Setting " << Key << endl;
  return( false );
}

bool GtkDasherStore::LoadSetting(const std::string& Key, std::string* Value)
{
  cout << "Load Setting " << Key <<  endl;
  return( false );
}
	
void GtkDasherStore::SaveSetting(const std::string& Key, bool Value)
{
  cout << "Save Setting " << Key << ", " << Value << endl;
}

void GtkDasherStore::SaveSetting(const std::string& Key, long Value)
{
  cout << "Save Setting " << Key << ", " << Value << endl;
}

void GtkDasherStore::SaveSetting(const std::string& Key, const std::string& Value)
{
  cout << "Save Setting " << Key << ", " << Value << endl;
}
