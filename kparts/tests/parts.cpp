
#include <kparts/event.h>

#include "parts.h"

#include <qcheckbox.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <q3multilineedit.h>
#include <qlineedit.h>
#include <q3vbox.h>

#include <kiconloader.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <klocale.h>

Part1::Part1( QObject *parent, QWidget * parentWidget )
 : KParts::ReadOnlyPart( parent )
{
    setObjectName( "Part1" );
  m_instance = new KInstance( "kpartstestpart" );
  setInstance( m_instance );
  m_edit = new Q3MultiLineEdit( parentWidget );
  setWidget( m_edit );
  setXMLFile( "kpartstest_part1.rc" );

  /*KAction * paBlah = */ //new KAction( "Blah", "filemail", 0, actionCollection(), "p1_blah" );
}

Part1::~Part1()
{
  delete m_instance;
}

bool Part1::openFile()
{
  kdDebug() << "Part1: opening " << QFile::encodeName(m_file) << endl;
  // Hehe this is from a tutorial I did some time ago :)
  QFile f(m_file);
  QString s;
  if ( f.open(QIODevice::ReadOnly) ) {
    QTextStream t( &f );
    while ( !t.atEnd() ) {
      s += t.readLine() + "\n";
    }
    f.close();
  } else
    return false;
  m_edit->setText(s);

  emit setStatusBarText( m_url.prettyURL() );

  return true;
}

Part2::Part2( QObject *parent, QWidget * parentWidget )
 : KParts::Part( parent )
{
  setObjectName( "Part2" );
  m_instance = new KInstance( "part2" );
  setInstance( m_instance );
  QWidget * w = new QWidget( parentWidget );
  w->setObjectName( "Part2Widget" );
  setWidget( w );

  QCheckBox * cb = new QCheckBox( "something", w );

  QLineEdit * l = new QLineEdit( "something", widget() );
  l->move(0,50);
  // Since the main widget is a dummy one, we HAVE to set
  // strong focus for it, otherwise we get the
  // the famous activating-file-menu-switches-part bug.
  w->setFocusPolicy( Qt::ClickFocus );

  // setXMLFile( ... ); // no actions currently
}

Part2::~Part2()
{
  delete m_instance;
}

void Part2::guiActivateEvent( KParts::GUIActivateEvent * event )
{
  if (event->activated())
    emit setWindowCaption("[part2 activated]");
}

#include "parts.moc"
