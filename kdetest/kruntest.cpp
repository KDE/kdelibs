// David Faure <faure@kde.org>
// Let's test KRun

#include <qpushbutton.h>
#include <kapp.h>
#include <kregistry.h>
#include <kregfactories.h> 
#include "kruntest.h"
 
void testKRun::foundMimeType( const char *_type )
{
  debug("testKRun::foundMimeType %s", _type);
  debug("testKRun::foundMimeType URL=%s", m_strURL.ascii());
  m_bFinished = true;
  m_timer.start( 0, true );
  return;
}

int main(int argc, char **argv)
{
  KApplication app( argc, argv );

  // The memory-eating solution will do.
  KRegistry registry;
  registry.addFactory( new KServiceTypeFactory );
  registry.addFactory( new KServiceFactory );
  registry.load();

  for (int i = 0 ; i < 1000 ; i++ )
  {
    debug(QString("creating testKRun %1").arg(i));
    testKRun * run1 = new testKRun( "file:/tmp", 0, true, true /* autodelete */ );
  }

  // A small UI to be able to terminate this correctly
        QWidget * w = new QWidget( );
        QPushButton h( "Press here to terminate", w );
        h.adjustSize();
        app.setMainWidget(w);
        w->show(); 
        QObject::connect( &h, SIGNAL(clicked()), &app, SLOT(quit()) );  

  // enter main orb/qt event loop
  app.exec();
}

#include "kruntest.moc"
