#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>

#include "resourcefactory.h"

using namespace KABC;

int main(int argc,char **argv)
{
    KAboutData aboutData("testkabc",I18N_NOOP("TestKabc"),"0.1");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;
	AddressBook ab;

	KConfig config("testmerc");

    ResourceFactory *factory = ResourceFactory::self();
	Resource *resource = factory->resource( "file", &ab, &config );
}
