#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>

#include "addressbook.h"
#include "plugins/file/resourcefile.h"
#include "formats/binaryformat.h"
#include "vcardformat.h"
#include "phonenumber.h"

using namespace KABC;

static const KCmdLineOptions options[] =
{
  { "save", "", 0 },
  { "number", "", 0 },
  KCmdLineLastOption
};

int main(int argc,char **argv)
{
  KAboutData aboutData("testaddressee","TestAddressee","0.1");
  KCmdLineArgs::init(argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions(options);

  KApplication app;
  KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

  kDebug() << "Creating a" << endl;
  Addressee a;
  
  kDebug() << "tick1" << endl;
  a.setGivenName("Hans");
  kDebug() << "tick2" << endl;
  a.setPrefix("Dr.");

  kDebug() << "Creating b" << endl;
  Addressee b( a );
  
  kDebug() << "tack1" << endl;
  a.setFamilyName("Wurst");
  kDebug() << "tack2" << endl;
  a.setNickName("hansi");

  kDebug() << "Creating c" << endl;
  Addressee c = a;
  
  kDebug() << "tock1" << endl;
  c.setGivenName("Eberhard");
  
  a.dump();  
  b.dump();
  c.dump();
}
