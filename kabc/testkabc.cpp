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
  KAboutData aboutData("testkabc",I18N_NOOP("TestKabc"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

//  KApplication app( false, false );
  KApplication app;
  
  AddressBook ab( new VCardFormat );
  
  ab.load( "/home/cs/kdecvs/kdepim/kabc/my.kabc" );
  kdDebug() << "Read addressbook from: " << ab.fileName() << endl;
  ab.dump();
  
  ab.clear();
  
  Addressee a;
  if ( a.isEmpty() ) kdDebug() << "1: a is empty" << endl;
  Addressee aa( a );
  if ( a.isEmpty() ) kdDebug() << "1: aa is empty" << endl;  
  a.setName( "Hans Speck" );
  if ( a.isEmpty() ) kdDebug() << "2: a is empty" << endl;
  Addressee aaa( a );
  if ( a.isEmpty() ) kdDebug() << "2: aaa is empty" << endl;  
  a.insertEmail( "hw@abc.de" );
  a.setBirthday( QDate( 1997, 4, 25 ) );
  ab.insertAddressee( a );

//  ab.dump();

  Addressee b;
//  b = a;
  b.setName( "Hilde Wurst" );
  b.insertPhoneNumber( PhoneNumber( "12345", PhoneNumber::Cell ) );
  ab.insertAddressee( b );

//  ab.dump();
  
  Addressee c( b );
  c.setName( "Klara Klossbruehe" );
  c.insertPhoneNumber( PhoneNumber( "00000", PhoneNumber::Cell ) );
  c.insertPhoneNumber( PhoneNumber( "4711", PhoneNumber::Fax ) );
  c.setNickName( "Klaerchen" );
  ab.insertAddressee( c );
  
//  ab.dump();
  
  AddressBook::Iterator it = ab.find( c );
  (*it).insertEmail( "neueemail@woauchimmer" );
  
  kdDebug() << "Write addressbook to: " << ab.fileName() << endl;
  ab.dump();
  
  AddressBook::Ticket *t = ab.requestSaveTicket( "/home/cs/kdecvs/kdepim/kabc/my.kabc" );
  if ( t ) {
    ab.save( t );
  } else {
    kdDebug() << "No ticket for save." << endl;
  }
}
