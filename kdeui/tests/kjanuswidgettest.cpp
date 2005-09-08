#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kjanuswidget.h>

#include <qstring.h>
#include <qcheckbox.h>
#include <q3vbox.h>

int main(int argc, char** argv)
{
  KCmdLineArgs::init( argc, argv, "januswidgettest", "JanusWidgetTest", "description", "version" );
  KApplication app;
  // -----
  KJanusWidget* janus = new KJanusWidget( 0, 0, KJanusWidget::IconList );

  Q3VBox* page1 = janus->addVBoxPage( QString( "Page1" ) ); // use i18n in real apps
  QCheckBox* cb1 = new QCheckBox( "a", page1 );

  Q3VBox* page2 = janus->addVBoxPage( QString( "Page2" ) );
  QCheckBox* cb2 = new QCheckBox( "a", page2 );

  janus->show();
  QObject::connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );

  return app.exec();
}

