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
  
  for( int i = 0; i < 1000; ++i ) {
    Addressee a;
    a.setGivenName( "number" + QString::number( i ) );
    a.setFamilyName( "Name" );
    a.insertEmail( QString::number( i ) + "@domain" );
    
    ab.insertAddressee( a );
  }
  printf( "\n" );
  
  AddressBook::Ticket *t = ab.requestSaveTicket( "my.kabc" );
  if ( t ) {
    ab.save( t );
  } else {
    kdDebug() << "No ticket for save." << endl;
  }
}
