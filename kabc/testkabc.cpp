#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>

#include "addressbook.h"
#include "resourcefile.h"
#include "binaryformat.h"
#include "vcardformat.h"
#include "phonenumber.h"

using namespace KABC;

static const KCmdLineOptions options[] =
{
	{ "save", I18N_NOOP("Show only process list of local host"), 0 },
	{ "number", I18N_NOOP("Show only process list of local host"), 0 },
	{ 0, 0, 0}
};

int main(int argc,char **argv)
{
    KAboutData aboutData("testkabc",I18N_NOOP("TestKabc"),"0.1");
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    AddressBook ab;
  
    ResourceFile r( &ab, locateLocal("data", "kabc/std.bin"), new BinaryFormat);
  
    if ( !ab.addResource( &r ) ) {
	kdDebug() << "Can't add Resource." << endl;
    }

    if (args->isSet("save"))
{
    for (uint i = 0; i < 4000; ++i) {
        Addressee a;
	a.setName("Tobias");
	a.setFamilyName("König");
	a.setGivenName("Tokoe");
	a.setAdditionalName("Willi");
	a.setPrefix("Dr. Prof.");
	a.setSuffix("III");
	a.setUrl("http://www.tokoeland.org");
	a.setNickName("tokoe");
	a.setMailer("mutt");
	a.setTitle("MyTitle");
	a.setRole("Entwickler");
	a.setOrganization("Klarälvdalens Data Consult AB");
	a.setProductId("unverkäuflich");
	a.setSortString("hmm sortieren");
	a.setBirthday( QDateTime( QDate(1997, 4, 25), QTime(17, 30, 42) ) );
	a.setNote("Wichtiger Hinweis\nDas ist eine Nachricht");
	a.setTimeZone(TimeZone(-1));
	a.setGeo(Geo(-19.5, 42.2));
	
	a.insertEmail( "tokoe@kde.org" );
	a.insertEmail( "tokoe82@yahoo.de" );
	a.insertEmail( "linuxcode@yahoo.de" );

	a.insertPhoneNumber(PhoneNumber("0351/2618572", PhoneNumber::Work));
	a.insertPhoneNumber(PhoneNumber("0351/2655692"));

	Address addr1;
	addr1.setType(Address::Home);
	addr1.setStreet("Borsbergstrasse 6");
	addr1.setPostalCode("01328");
	addr1.setRegion("Saxony");
	addr1.setCountry("Germany");
	addr1.setExtended("Erweitert");
	addr1.setLocality("Uni Dresden");
	a.insertAddress(addr1);

	Address addr2;
	addr2.setType(Address::Work);
	addr2.setStreet("Bärensteiner Straße 7");
	addr2.setPostalCode("01288");
	addr2.setRegion("Saxony");
	addr2.setCountry("Germany");
	addr2.setExtended("Nothing");
	addr2.setLocality("Internet24");
	a.insertAddress(addr2);

	a.insertCategory("FooCategory1");
	a.insertCategory("BarCategory2");
	a.insertCategory("XBFCategory3");


	a.insertCustom("testkabc", "testvalue", "foo");
	a.insertCustom("testkabc", "testvalue2", "bar");

	ab.insertAddressee( a );
    }

    Ticket *t = ab.requestSaveTicket( &r );
    if ( t ) {
    	ab.save( t );
    } else {
	kdDebug() << "No ticket for save." << endl;
    }
} else {
    if (!(ab.load()))
	kdDebug() << "can't open file" << endl;

    ab.dump();
}
}
