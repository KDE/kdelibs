// David Faure <faure@kde.org>
// Let's test KRun

#include <qpushbutton.h>
#include <kapp.h>
#include "kruntest.h"
 
const int MAXKRUNS = 100;

testKRun * myArray[MAXKRUNS];

void testKRun::foundMimeType( const char *_type )
{
  debug("testKRun::foundMimeType %s", _type);
  debug("testKRun::foundMimeType URL=%s", m_strURL.ascii());
  m_bFinished = true;
  m_timer.start( 0, true );
  return;
}

Receiver::Receiver()
{
        QPushButton * h = new QPushButton( "Press here to terminate", this );
        h->adjustSize();
        h->move(0,0);
        start = new QPushButton( "Launch KRuns", this );
        start->adjustSize();
        stop = new QPushButton( "Stop those KRuns", this );
        stop->adjustSize();
        stop->setEnabled(false);
        start->move(0,50);
        stop->move(0,100);
        QObject::connect( h, SIGNAL(clicked()), kapp, SLOT(quit()) );  
        QObject::connect( start, SIGNAL(clicked()), this, SLOT(slotStart()) );
        QObject::connect( stop, SIGNAL(clicked()), this, SLOT(slotStop()) );

        show(); 
}

void Receiver::slotStop() 
{ 
  for (int i = 0 ; i < MAXKRUNS ; i++ )
  {
    debug(" deleting krun %i",i);
    delete myArray[i];
  }
  start->setEnabled(true);
  stop->setEnabled(false);
}


void Receiver::slotStart()
{
  for (int i = 0 ; i < MAXKRUNS ; i++ )
  {
    debug(QString("creating testKRun %1").arg(i));
    myArray[i] = new testKRun( "file:/tmp", 0, true, false /* no autodelete */ );
  }
  start->setEnabled(false);
  stop->setEnabled(true);
}

int main(int argc, char **argv)
{
  KApplication app( argc, argv );
  Receiver receiver;

  app.setMainWidget(&receiver);

  return app.exec();
}

#include "kruntest.moc"
