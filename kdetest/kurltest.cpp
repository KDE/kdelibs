#include <iostream.h>
#include "kurl.h"

static void url_info( KURL url)
{     
     cout << "------------------------------------\n";
     cout << "URL: " << url.url() << "\n";
     cout << "Protocol: " << url.protocol() << "\n";
     cout << "Path: " << url.path() << "\n";
     cout << "Host: " << url.host() << "\n";
     cout << "Reference: " << url.reference() << "\n";
}

int
main( int argc, char** argv)
{
  int i = 0;

     if( argc != 2) {
	  cout << "Usage: " << argv[0] << " url\n";
	  return 1;
     }
     KURL url( argv[1]);
     if( url.isMalformed()) {
	  cout << argv[1] << " is not a valid URL\n";
	  return 1;
     }
     url.cleanURL();
     url_info( url);
     cout << "cd to bleb, then .. 3 times\n";
     url.cd( "bleb");
     url_info( url);
     for( i = 0; i < 3; ++i) {
	  url.cdUp();
	  url_info( url);
     }
     cout << "add blabla to reference then .. 3 times\n";
     url.cd( "blabla", 1, 1);
     url_info( url);
     for( i = 0; i < 3; ++i) {
	  url.cdRef("..");
	  url_info( url);
     }
     cout << "change protocol to tar\n";
     url.setProtocol( "tar");
     cout << "add blabla to reference\n";
     url.cd( "blabla", 1, 1);
     url_info( url);
     cout << "remove reference\n";
     url.setReference( "");
     url_info( url);
     cout << "setting reference to stuff\n";
     url.setReference( "stuff");
     url_info( url);
     return 0;
}
