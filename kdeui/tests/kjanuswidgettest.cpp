#include <kapplication.h>
#include <kjanuswidget.h>

#include <qstring.h>
#include <qcheckbox.h>
#include <qvbox.h>

int main(int argc, char** argv)
{
  KApplication app(argc, argv, "JanusWidgetTest");
  // -----
  KJanusWidget* janus = new KJanusWidget( 0, 0, KJanusWidget::IconList );

  QVBox* page1 = janus->addVBoxPage( QString( "Page1" ) ); // use i18n in real apps
  QCheckBox* cb1 = new QCheckBox( "a", page1 );

  QVBox* page2 = janus->addVBoxPage( QString( "Page2" ) );
  QCheckBox* cb2 = new QCheckBox( "a", page2 );

  janus->show();
  QObject::connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );

  return app.exec();
}

