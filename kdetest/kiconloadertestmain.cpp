
#include <qpushbutton.h>
#include <qlineedit.h>

#include <kapp.h>
#include <kiconloader.h>
#include <kiconloaderdialog.h>

#include "kiconloadertestmain.h"
#include <kglobal.h>
#include "kiconloadertestmain.h"

MyMain::MyMain()
{
  icon_sel = new KIconLoaderDialog;
  test = new QPushButton("test", this);
  test->setPixmap(ICON("exit.xpm"));
  test->setGeometry(0,0,50,50);
  connect(test, SIGNAL(clicked()), this, SLOT(changePix()));
  le_dir = new QLineEdit(this);
  le_dir->setGeometry( 10, 70, 300, 24 );
  connect( le_dir, SIGNAL(returnPressed()), this, SLOT(insertPath()) );
  le_dir->show();
  test->show();
}

void MyMain::changePix()
{
  QString name;
  QPixmap pix = icon_sel->selectIcon( name, "*" );
  if( !pix.isNull() )
    {
      debug("name = %s", name.ascii() );
      test->setPixmap(pix);
    }
}

void MyMain::insertPath()
{
  KGlobal::iconLoader()->insertDirectory(3, le_dir->text() );
  delete icon_sel;
  icon_sel = new KIconLoaderDialog();
}
