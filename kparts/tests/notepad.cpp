
#include "notepad.h"

#include <qsplitter.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qmultilinedit.h>

#include <kapp.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <klocale.h>
#include <kstatusbar.h>

NotepadPart::NotepadPart( QWidget * parentWidget )
 : KReadWritePart( "NotepadPart" )
{
  m_edit = new QMultiLineEdit( parentWidget );
  setWidget( m_edit );
  (void)new KAction( i18n( "Search and replace" ), 0, actionCollection(), "searchreplace" );
}

NotepadPart::~NotepadPart()
{
}

bool NotepadPart::openFile()
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
  }
  m_edit->setText(s);

  if ( m_host )
  {
    m_host->setWindowCaption( m_url.url() );
    
    ((KStatusBar *)m_host->topLevelContainer( "StatusBar" ))->message( m_url.url() );
  }

  return true;
}

QString NotepadPart::configFile() const
{
  // Hmm...
  return KXMLGUIFactory::readConfigFile( "notepadpart.rc" );
}

#include "notepad.moc"
