#include "parts.h"

#include <kpartmanager.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qmultilinedit.h>
#include <qlineedit.h>
#include <qvbox.h>

#include <kiconloader.h>
#include <kapp.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <klocale.h>

Part1::Part1( QObject *parent, QWidget * parentWidget )
 : KParts::ReadOnlyPart( parent, "Part1" )
{
  m_instance = new KInstance( "kpartstestpart" );
  setInstance( m_instance );
  m_edit = new QMultiLineEdit( parentWidget );
  setWidget( m_edit );
  setXMLFile( "kpartstest_part1.rc" );

  KAction * paBlah = new KAction( i18n( "Blah" ), 0, actionCollection(), "p1_blah" );
  paBlah->setIconSet(QIconSet(BarIcon("filemail")));
}

Part1::~Part1()
{
  delete m_instance;
}

bool Part1::openFile()
{
  debug("Part1: opening %s", m_file.ascii());
  // Hehe this is from a tutorial I did some time ago :)
  QFile f(m_file);
  QString s;
  if ( f.open(IO_ReadOnly) ) {
    QTextStream t( &f );
    while ( !t.eof() ) {
      s += t.readLine() + "\n";
    }
    f.close();
  } else
    return false;
  m_edit->setText(s);

  if (manager())
    manager()->setWindowCaption( m_url.url() );
  //See comments
  //manager()->statusBar()->message( m_url.url() );

  return true;
}

Part2::Part2( QObject *parent, QWidget * parentWidget )
 : KParts::Part( parent, "Part2" )
{
  m_instance = new KInstance( "part2" );
  setInstance( m_instance );
  QWidget * w = new QWidget( parentWidget, "Part2Widget" );
  setWidget( w );

  QCheckBox * cb = new QCheckBox( "something", w );

  QLineEdit * l = new QLineEdit( "something", widget() );
  l->move(0,50);
  // Since the main widget is a dummy one, we HAVE to set
  // strong focus for it, otherwise we get the
  // the famous activating-file-menu-switches-part bug.
  w->setFocusPolicy( QWidget::ClickFocus );

  // setXMLFile( ... ); // no actions currently
}

Part2::~Part2()
{
  delete m_instance;
}

#include "parts.moc"
