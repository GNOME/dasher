// GtkDasherStore.cc
// (c) 2002 Philip Cowans

#include "Gtk2DasherStore.h"
#include <iostream>
#include <fstream>

#include <map>
#include <stdio.h>

#include <sys/stat.h>

GtkDasherStore::GtkDasherStore()
{
  create_rcdir();
  read_from_file();
}

GtkDasherStore::~GtkDasherStore()
{
}

bool GtkDasherStore::LoadSetting(const std::string& Key, bool* Value)
{
  *Value = bmap[Key];

  if( *Value == bool() )
    return( false );
  else
    return( true );
}

bool GtkDasherStore::LoadSetting(const std::string& Key, long* Value)
{
  *Value = lmap[Key];

  if( *Value == long() )
    return( false );
  else
    return( true );
}

bool GtkDasherStore::LoadSetting(const std::string& Key, std::string* Value)
{
  *Value = smap[Key];

  if( *Value == std::string() )
    return( false );
  else
    return( true );
}
	
void GtkDasherStore::SaveSetting(const std::string& Key, bool Value)
{
  if( bmap[Key] != Value )
    {
      bmap[Key] = Value;
      write_to_file();
    }
}

void GtkDasherStore::SaveSetting(const std::string& Key, long Value)
{
  if( lmap[Key] != Value )
    {
      lmap[Key] = Value;
      write_to_file();
    }
}

void GtkDasherStore::SaveSetting(const std::string& Key, const std::string& Value)
{
  if( smap[Key] != Value )
    {
      smap[Key] = Value;
      write_to_file();
    }
}

void GtkDasherStore::write_to_file()
{
  std::ofstream outfile;

  char *HomeDir;

  HomeDir = getenv( "HOME" );

  char *UserDataDir;

  UserDataDir = new char[ strlen( HomeDir ) + 18 ];
  sprintf( UserDataDir, "%s/.dasher/dasherrc", HomeDir );

  outfile.open(UserDataDir);

  if( !outfile.bad() )
  {
    {
      std::map<std::string, bool>::iterator bit;
      
      bit = bmap.begin();
      
      while( bit != bmap.end() )
	{
	  outfile << "b:" << bit->first << ":" << bit->second << std::endl;
	  ++bit;
	}
    }
    
    { 
      std::map<std::string, long>::iterator lit;
      
      lit = lmap.begin();
      
      while( lit != lmap.end() )
	{
	  outfile << "l:" << lit->first << ":" << lit->second << std::endl;
	  ++lit;
	}
    } 
    
    { 
      std::map<std::string, std::string>::iterator sit;
      
      sit = smap.begin();
      
      while( sit != smap.end() )
	{
	  outfile << "s:" << sit->first << ":" << sit->second << std::endl;
	  ++sit;
	}
    }
    
    outfile.close();
  }
  else
    std::cerr << "Warning - failed to save configuration data" << std::endl;
  
  delete( UserDataDir );
}

void GtkDasherStore::read_from_file()
{
 std::ifstream infile;

  char *HomeDir;

  HomeDir = getenv( "HOME" );

  char *UserDataDir;

  UserDataDir = new char[ strlen( HomeDir ) + 18 ];
  sprintf( UserDataDir, "%s/.dasher/dasherrc", HomeDir );

  infile.open( UserDataDir );
 
 if( infile.good() )
   {
     char ibuffer[256];

     infile.getline( ibuffer, 256 );

     while( !infile.eof() )
       {
	 std::string foo(ibuffer);

	 int pos1;
	 int pos2;

	 pos1 = foo.find(':');
	 pos2 = foo.find(':',pos1+1);

	 std::string key( foo.substr( pos1+1, pos2-pos1-1 ) );
	 std::string value( foo.substr( pos2+1 ));

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
	     std::cerr << "Waring - possibly corrupt configuration file" << std::endl;
	     break;
	   }
	 infile.getline( ibuffer, 256 );
       }
   }

 delete( UserDataDir );
 
}

void GtkDasherStore::create_rcdir()
{
  // Create a .dasher directory if it doesn't already exist

  char *HomeDir;

  HomeDir = getenv( "HOME" );

  char *UserDataDir;

  UserDataDir = new char[ strlen( HomeDir ) + 9 ];
  sprintf( UserDataDir, "%s/.dasher", HomeDir );

  mkdir( UserDataDir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );

  delete( UserDataDir );
  delete( HomeDir );
}













