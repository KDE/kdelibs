#include "ksqueezedtextlabel.h"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kvbox.h>

int main( int argc, char **argv )
{
  KCmdLineArgs::init(argc, argv, "test", "Test", "test app", "1.0");
  KApplication app;

  KVBox* box = new KVBox();
  KSqueezedTextLabel *l = new KSqueezedTextLabel( "This is a rather long string", box);
  l = new KSqueezedTextLabel( "This is another long string", box );
  l->setTextElideMode( Qt::ElideRight );
  box->show();

  return app.exec();
}
