#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>

#include "geo.h"
#include "secrecy.h"
#include "stdaddressbook.h"
#include "timezone.h"

using namespace KABC;

int main(int argc,char **argv)
{
    KAboutData aboutData("testkabc",I18N_NOOP("TestKabc"),"0.1");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;
    AddressBook *ab = StdAddressBook::self();

    Addressee addr;

    addr.setGivenName("Tobias");
    addr.setFamilyName("Koenig");
    addr.setSecrecy( Secrecy( Secrecy::Confidential ) );

    TimeZone zone;
    zone.setOffset( -188 );
    addr.setTimeZone( zone );

    Geo geo;
    geo.setLatitude( 19.2233224 );
    geo.setLongitude( -20.2233224 );
    addr.setGeo( geo );

    ab->insertAddressee( addr );

    StdAddressBook::save();

    return 0;
}
