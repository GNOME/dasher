#include <sys/socket.h>
#include <sys/un.h>

#include <cmath>
#include <cctype>
#include <algorithm>
#include <string>

using namespace std;

int main(int argc, char **argv) {

  // Create a socket to listen for requests

  int skt;

  skt = socket( PF_UNIX, SOCK_STREAM, 0 );

  sockaddr_un sa;
  
  string sktaddr( "/tmp/.dasherd.socket" );

  unlink( sktaddr.c_str() );

  sa.sun_family = AF_UNIX;
  strcpy( sa.sun_path, sktaddr.c_str() );

  int rv;

  rv = bind( skt, (sockaddr *)(&sa), sizeof( sa ) );

  if( rv == -1 ) {
    cout << "strerror says: " << strerror( errno )<< endl;
    exit(1);
  }

  // Make sure the permissions are right on the socket

  chmod( sktaddr.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );

  rv = listen( skt, 10 );

  if( rv == -1 ) {
    cout << "strerror says: " << strerror( errno )<< endl;
    exit(1);
  }

  sockaddr_un con;
  char buffer[1024];


  while( true ) {
    socklen_t conl( sizeof( con ));

    int newskt;
    
    newskt = accept( skt, (sockaddr *)(&con), &conl );
    
    cout << "New socket is " << newskt << endl;
    
    if( newskt == -1 ) {
      cout << "strerror says: " << strerror( errno )<< endl;
      exit(1);
    }
    
    int rcvd( recv( newskt, buffer, 1024, 0 ) );
    
    string msg( buffer, rcvd );
    
    cout << "Message received: " << msg << endl;
    close( newskt );
    
  }

}
