
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
  debug("NotepadPart::NotepadPart");
  m_edit = new QMultiLineEdit( parentWidget, "NotepadPart's multiline edit" );
  m_edit->show(); // don't forget this !
  setWidget( m_edit );
  (void)new KAction( i18n( "Search and replace" ), 0, actionCollection(), "searchreplace" );
  // TODO connect m_edit->changed to setModified()
}

NotepadPart::~NotepadPart()
{
}

bool NotepadPart::openFile()
{
  debug("NotepadPart: opening %s", m_file.ascii());
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

bool NotepadPart::save()
{
  QFile f(m_file);
  QString s;
  if ( f.open(IO_WriteOnly) ) {
    QTextStream t( &f );
    t << m_edit->text();
    f.close();
  } else
    return false;
  return saveToURL();
}

QString NotepadPart::configFile() const
{
  // Hmm...
  return KXMLGUIFactory::readConfigFile( "notepadpart.rc" );
}

#include "notepad.moc"
