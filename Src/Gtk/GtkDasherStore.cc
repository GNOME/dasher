#include "GtkDasherStore.h"
#include <iostream.h>
#include <fstream.h>

#include <map>

GtkDasherStore::GtkDasherStore()
{
  cout << "Warning - this version of Dasher has been built without support for storage" << endl << "of configuration settings. Changes made to the configuration will be lost" << endl << "when you exit Dasher." << endl;
  
  read_from_file();
}

GtkDasherStore::~GtkDasherStore()
{
}

bool GtkDasherStore::LoadSetting(const std::string& Key, bool* Value)
{
  //  cout << "Load Setting " << Key <<  endl;

  *Value = bmap[Key];

  if( *Value == bool() )
    return( false );
  else
    return( true );
}

bool GtkDasherStore::LoadSetting(const std::string& Key, long* Value)
{
  //  cout << "Load Setting " << Key << endl;
  *Value = lmap[Key];

  if( *Value == long() )
    return( false );
  else
    return( true );
}

bool GtkDasherStore::LoadSetting(const std::string& Key, std::string* Value)
{
  //  cout << "Load Setting " << Key <<  endl;
  *Value = smap[Key];

  if( *Value == string() )
    return( false );
  else
    return( true );
}
	
void GtkDasherStore::SaveSetting(const std::string& Key, bool Value)
{
  //  cout << "Save Setting " << Key << ", " << Value << endl;
  
    bmap[Key] = Value;
   write_to_file();
}

void GtkDasherStore::SaveSetting(const std::string& Key, long Value)
{
  //  cout << "Save Setting " << Key << ", " << Value << endl;

  lmap[Key] = Value;

  write_to_file();
}

void GtkDasherStore::SaveSetting(const std::string& Key, const std::string& Value)
{
  //  cout << "Save Setting " << Key << ", " << Value << endl;

  smap[Key] = Value;

  write_to_file();
}

void GtkDasherStore::write_to_file()
{
  ofstream outfile;

  outfile.open("/home/pjc51/.dasher/dasherrc");

  if( !outfile.bad() )
  {
  {
    std::map<std::string, bool>::iterator bit;
    
    bit = bmap.begin();
    
    while( bit != bmap.end() )
      {
	outfile << "b:" << bit->first << ":" << bit->second << endl;
	++bit;
      }
  }
   
  { 
    std::map<std::string, long>::iterator lit;
    
    lit = lmap.begin();
    
    while( lit != lmap.end() )
      {
	outfile << "l:" << lit->first << ":" << lit->second << endl;
	++lit;
      }
  } 

  { 
    std::map<std::string, string>::iterator sit;
    
    sit = smap.begin();
    
    while( sit != smap.end() )
      {
	outfile << "s:" << sit->first << ":" << sit->second << endl;
	++sit;
      }
  }

  outfile.close();
}
  else
    cerr << "Warning - failed to save configuration data" << endl;
}

void GtkDasherStore::read_from_file()
{
 ifstream infile;

 infile.open("/home/pjc51/.dasher/dasherrc");
 
 if( !infile.bad() )
   {
     char ibuffer[256];

     infile.getline( ibuffer, 256 );

     while( !infile.eof() )
       {
	 cout << "Read: " << ibuffer << endl;
	 string foo(ibuffer);

	 int pos1;
	 int pos2;

	 pos1 = foo.find(':');
	 pos2 = foo.find(':',pos1+1);

	 cout << pos1 << ", " << pos2 << endl;

	 string key( foo.substr( pos1+1, pos2-pos1-1 ) );
	 string value( foo.substr( pos2+1 ));

	 cout << key << " " << value << endl;

	 switch( foo[0] )
	   {
	   case 'b':
	     bmap[ key ] = atoi( value.c_str() );
	     break;
	   case 'l':
	     lmap[ key ] = atol( value.c_str() );
	     break;
	   case 's':
	     smap[ key ] = value;
	     break;
	   default:
	     cerr << "Waring - possibly corrupt configuration file" << endl;
	     break;
	   }


	 infile.getline( ibuffer, 256 );
       }
   }
 
}
