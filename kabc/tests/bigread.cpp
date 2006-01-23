#include <sys/times.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "addressbook.h"
#include "vcardformat.h"
#include "plugins/file/resourcefile.h"
#if 0
#include "resourcesql.h"
#endif

using namespace KABC;

int main(int argc,char **argv)
{
  KAboutData aboutData("bigread","BigReadKabc","0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app( false);
  
  AddressBook ab; 
   
  ResourceFile r( "my.kabc", "vcard2" );
  ab.addResource( &r );

#if 0  
  ResourceSql rsql( &ab, "root", "kde4ever", "localhost" );
  ab.addResource( &rsql );
#endif

  struct tms start;

  times( &start );

#if 0
  kdDebug() << "utime : " << int( start.tms_utime ) << endl;
  kdDebug() << "stime : " << int( start.tms_stime ) << endl;
  kdDebug() << "cutime: " << int( start.tms_cutime ) << endl;
  kdDebug() << "cstime: " << int( start.tms_cstime ) << endl;
#endif
	    
  kdDebug() << "Start load" << endl;
  ab.load();
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

//  ab.dump();
}
