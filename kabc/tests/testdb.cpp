#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "addressbook.h"
#include "vcardformat.h"
#include "resourcesql.h"

using namespace KABC;

int main(int argc,char **argv)
{
  KAboutData aboutData("testdb","TestKabcDB","0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

//  KApplication app( false, false );
  KApplication app;

  AddressBook ab;
  
  ResourceSql r( &ab, "root", "kde4ever", "localhost" );
  if ( ! r.open() ) {
    kDebug() << "Failed to open resource." << endl;
  }
  
  r.load( &ab );
  
  r.close();
  
  ab.dump();
}
