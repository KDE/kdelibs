#include "ksqueezedtextlabel.h"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kvbox.h>

int main( int argc, char **argv )
{
  KCmdLineArgs::init(argc, argv, "test", 0, ki18n("Test"), "1.0", ki18n("test app"));
  KApplication app;

  KVBox* box = new KVBox();
  KSqueezedTextLabel *l = new KSqueezedTextLabel( "This is a rather long string", box);
  l = new KSqueezedTextLabel( "This is another long string", box );
  l->setTextElideMode( Qt::ElideRight );
  box->show();

  return app.exec();
}
