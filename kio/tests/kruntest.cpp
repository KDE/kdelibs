// David Faure <faure@kde.org>
// Let's test KRun

#include <qpushbutton.h>
#include <kapplication.h>
#include "kruntest.h"
#include <kdebug.h>
#include <stdlib.h>

const int MAXKRUNS = 100;

testKRun * myArray[MAXKRUNS];

void testKRun::foundMimeType( const QString& _type )
{
  kdDebug() << "testKRun::foundMimeType " << _type << endl;
  kdDebug() << "testKRun::foundMimeType URL=" << m_strURL.url() << endl;
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
    kdDebug() << " deleting krun " << i << endl;
    delete myArray[i];
  }
  start->setEnabled(true);
  stop->setEnabled(false);
}


void Receiver::slotStart()
{
  for (int i = 0 ; i < MAXKRUNS ; i++ )
  {
    kdDebug() << "creating testKRun " << i << endl;
    myArray[i] = new testKRun( KURL("file:/tmp"), 0, true, false /* no autodelete */ );
  }
  start->setEnabled(false);
  stop->setEnabled(true);
}

bool check(QString txt, QString a, QString b)
{
  if (a.isEmpty())
     a = QString::null;
  if (b.isEmpty())
     b = QString::null;
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

int main(int argc, char **argv)
{
  KApplication app( argc, argv, "kruntest", true /* it _has_ a GUI ! */);

  // First some non-interactive tests
  check( "binaryName('/usr/bin/ls', true)", KRun::binaryName("/usr/bin/ls", true), "ls");
  check( "binaryName('/usr/bin/ls', false)", KRun::binaryName("/usr/bin/ls", false), "/usr/bin/ls");
  check( "binaryName('/path/to/wine \"long argument with path\"')", KRun::binaryName("/path/to/wine \"long argument with path\"", true), "wine" );
  // TODO check( "binaryName('/path/with/a/sp\\ ace/exe arg1 arg2')", KRun::binaryName("/path/with/a/sp\\ ace/exe arg1 arg2", true), "exe" );
  check( "binaryName('\"progname\" \"arg1\"')", KRun::binaryName("\"progname\" \"arg1\"", true), "progname" );
  check( "binaryName('\'quoted\' \"arg1\"')", KRun::binaryName("'quoted' \"arg1\"", true), "quoted" );

  Receiver receiver;

  app.setMainWidget(&receiver);

  return app.exec();
}

#include "kruntest.moc"
