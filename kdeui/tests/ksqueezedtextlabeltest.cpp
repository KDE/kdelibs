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
  l = new KSqueezedTextLabel( "This is another long string, selectable by mouse", box );
  l->setTextElideMode( Qt::ElideRight );
  l->setTextInteractionFlags(Qt::TextSelectableByMouse);

  new QLabel("This is a normal QLabel", box);
  QLabel* selectableLabel = new QLabel("This is a QLabel, selectable by mouse", box);
  selectableLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

  box->show();

  return app.exec();
}
