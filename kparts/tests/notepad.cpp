
#include "notepad.h"
#include <kpartmanager.h>

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
#include <kstddirs.h>

NotepadPart::NotepadPart( QWidget * parentWidget )
 : KReadWritePart( "NotepadPart" )
{
  m_instance = new KInstance( "nodepadpart" );

  m_instance->dirs()->addResourceDir( "appdata", QDir::currentDirPath().append( "/notepadplugins" ).ascii() );

  debug("NotepadPart::NotepadPart");
  m_edit = new QMultiLineEdit( parentWidget, "NotepadPart's multiline edit" );
  m_edit->setFocus();
  setWidget( m_edit );
  setXMLFile( "notepadpart.rc" );
  (void)new KAction( i18n( "Search and replace" ), 0, actionCollection(), "searchreplace" );
  // TODO connect m_edit->changed to setModified()
}

NotepadPart::~NotepadPart()
{
  delete m_instance;
}

KInstance *NotepadPart::instance()
{
  return m_instance;
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

  manager()->setWindowCaption( m_url.url() );
  // see comments
  //manager()->statusBar()->message( m_url.url() );

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

#include "notepad.moc"
