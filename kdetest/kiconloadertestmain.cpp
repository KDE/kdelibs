
#include <qpushbt.h>

#include <kiconloader.h>

extern KIconLoader *global_icon_loader;

#include "kiconloadertestmain.h"
#include "kiconloadertestmain.moc"

MyMain::MyMain()
{
  initMetaObject();
  test = new QPushButton("test", this);
  test->setPixmap(global_icon_loader->loadIcon("exec.xpm"));
  test->setGeometry(0,0,50,50);
  connect(test, SIGNAL(clicked()), this, SLOT(changePix()));
  test->show();
  global_icon_loader->setCaching(TRUE);
}

void MyMain::changePix()
{
  QString name;
  QPixmap pix = global_icon_loader->selectIcon( name, "*" );
  if( !pix.isNull() )
    {
      debug("name = %s", (const char *) name );
      test->setPixmap(pix);
    }
}
