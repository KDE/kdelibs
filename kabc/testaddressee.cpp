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
  { "save", "", 0 },
  { "number", "", 0 },
  { 0, 0, 0}
};

int main(int argc,char **argv)
{
  KAboutData aboutData("testaddressee","TestAddressee","0.1");
  KCmdLineArgs::init(argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions(options);

  KApplication app;
  KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

  kdDebug() << "Creating a" << endl;
  Addressee a;
  
  kdDebug() << "tick1" << endl;
  a.setGivenName("Hans");
  kdDebug() << "tick2" << endl;
  a.setPrefix("Dr.");

  kdDebug() << "Creating b" << endl;
  Addressee b( a );
  
  kdDebug() << "tack1" << endl;
  a.setFamilyName("Wurst");
  kdDebug() << "tack2" << endl;
  a.setNickName("hansi");

  kdDebug() << "Creating c" << endl;
  Addressee c = a;
  
  kdDebug() << "tock1" << endl;
  c.setGivenName("Eberhard");
  
  a.dump();  
  b.dump();
  c.dump();
}
