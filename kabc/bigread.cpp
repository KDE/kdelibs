#include <sys/times.h>

#include <kaboutdata.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "addressbook.h"
#include "vcardformat.h"

using namespace KABC;

int main(int argc,char **argv)
{
  KAboutData aboutData("bigtest","BigTestKabc","0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;
  
  AddressBook ab( new VCardFormat );

  struct tms start;

  times( &start );

#if 0
  kdDebug() << "utime : " << int( start.tms_utime ) << endl;
  kdDebug() << "stime : " << int( start.tms_stime ) << endl;
  kdDebug() << "cutime: " << int( start.tms_cutime ) << endl;
  kdDebug() << "cstime: " << int( start.tms_cstime ) << endl;
#endif
	    
  kdDebug() << "Start load" << endl;
  ab.load( "my.kabc" );
  kdDebug() << "Finished load" << endl;

  struct tms end;

  times( &end );

#if 0
  kdDebug() << "utime : " << int( end.tms_utime ) << endl;
  kdDebug() << "stime : " << int( end.tms_stime ) << endl;
  kdDebug() << "cutime: " << int( end.tms_cutime ) << endl;
  kdDebug() << "cstime: " << int( end.tms_cstime ) << endl;
#endif

  kdDebug() << "UTime: " << int( end.tms_utime ) - int( start.tms_utime ) << endl; 
  kdDebug() << "STime: " << int( end.tms_stime ) - int( start.tms_stime ) << endl; 
}
