
#include "notepad.h"
#include <kpartmanager.h>
#include <kpartsmainwindow.h>
#include <kxmlgui.h>

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
 : KParts::ReadWritePart( "NotepadPart" )
{
  m_instance = new KInstance( "notepadpart" );

  debug("NotepadPart::NotepadPart");
  m_edit = new QMultiLineEdit( parentWidget, "NotepadPart's multiline edit" );
  m_edit->setFocus();
  setWidget( m_edit );
  setXMLFile( "notepadpart.rc" );
  (void)new KAction( i18n( "Search and replace" ), 0, this, SLOT( slotSearchReplace() ), actionCollection(), "searchreplace" );
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

void NotepadPart::slotSearchReplace()
{
  KParts::XMLGUIFactory *factory = servant()->factory();

  if ( !factory )
    return;

  QListIterator<KParts::XMLGUIServant> it( *pluginServants() );
  for (; it.current(); ++it )
    factory->removeServant( it.current() );
}

#include "notepad.moc"
