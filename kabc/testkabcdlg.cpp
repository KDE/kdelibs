#include <qwidget.h>

#include <kaboutdata.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "addresseedialog.h"

using namespace KABC;

int main(int argc,char **argv)
{
  KAboutData aboutData("testkabc",I18N_NOOP("TestKabc"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;

  Addressee a = AddresseeDialog::getAddressee( 0 );
  
  if ( !a.isEmpty() ) {
    kdDebug() << "Selected Addressee:" << endl;
    a.dump();
  } else {
    kdDebug() << "No Addressee selected." << endl;
  }
}
