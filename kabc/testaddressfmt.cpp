#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>

#include "addressbook.h"
#include "address.h"

using namespace KABC;

static const KCmdLineOptions options[] =
{
  { "save", "", 0 },
  { "number", "", 0 },
  { 0, 0, 0}
};

int main(int argc,char **argv)
{
  KAboutData aboutData("testaddressfmt","TestAddressFormat","0.1");
  KCmdLineArgs::init(argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions(options);

  KApplication app;

  Address a;
  a.setStreet("Lummerlandstr. 1");
  a.setPostalCode("12345");
  a.setLocality("Lummerstadt");
  a.setCountry ("Germany");

  Address b;
  b.setStreet("457 Foobar Ave");
  b.setPostalCode("1A2B3C");
  b.setLocality("Nervousbreaktown");
  b.setRegion("DC");
  b.setCountry("United States of America");

  Address c;
  c.setStreet("Lummerlandstr. 1");
  c.setPostalCode("12345");
  c.setLocality("Lummerstadt");
  c.setCountry ("Deutschland");

  Address d;
  d.setStreet("Lummerlandstr. 1");
  d.setPostalCode("12345");
  d.setLocality("Lummerstadt");
  d.setCountry ("");

  qDebug( "-------------------------------------\nShould have german address formatting, local country formatting\n" );
  qDebug( a.formattedAddress("Jim Knopf").latin1() );
  qDebug( "-------------------------------------\nShould have US address formatting, local country formatting\n" );
  qDebug( b.formattedAddress("Huck Finn").latin1() );
  qDebug( "-------------------------------------\nShould have german address formatting, local country formatting\n" );
  qDebug( c.formattedAddress("Jim Knopf").latin1() );
  qDebug( "-------------------------------------\nShould have local address formatting, local country formatting\n" );
  qDebug( d.formattedAddress("Jim Knopf").latin1() );
}


