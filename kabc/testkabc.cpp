#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>

#include "resourcefile.h"
#include "vcardformat.h"

using namespace KABC;

int main(int argc,char **argv)
{
    KAboutData aboutData("testkabc",I18N_NOOP("TestKabc"),"0.1");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;
    AddressBook ab;
    Addressee addr;

    Resource *resource = new ResourceFile( &ab, "/tmp/std.vcf", new VCardFormat );

    ab.addResource( resource );

    Geo geo;
    geo.setLatitude(19.821907);
    geo.setLongitude(-42.190782);

    addr.setGivenName("Tobias");
    addr.setFamilyName("Koenig");
    addr.setGeo( geo );

    ab.insertAddressee( addr );

    Ticket *ticket = ab.requestSaveTicket( resource );
    ab.save( ticket );

    return 0;
}
