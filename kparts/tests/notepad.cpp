
#include "notepad.h"
#include <kparts/partmanager.h>
#include <kparts/mainwindow.h>

#include <qsplitter.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kstandarddirs.h>

NotepadPart::NotepadPart( QWidget* parentWidget, const char*,
                          QObject* parent, const char* name,
                          const QStringList& )
 : KParts::ReadWritePart( parent, name )
{
  setInstance( NotepadFactory::instance() );

  m_edit = new QMultiLineEdit( parentWidget, "NotepadPart's multiline edit" );
  setWidget( m_edit );

  (void)new KAction( "Search and replace", 0, this, SLOT( slotSearchReplace() ), actionCollection(), "searchreplace" );
  setXMLFile( "notepadpart.rc" );
  setReadWrite( true );
}

NotepadPart::~NotepadPart()
{
}

void NotepadPart::setReadWrite( bool rw )
{
    m_edit->setReadOnly( !rw );
    if (rw)
        connect( m_edit, SIGNAL( textChanged() ), this, SLOT( setModified() ) );
    else
        disconnect( m_edit, SIGNAL( textChanged() ), this, SLOT( setModified() ) );

    ReadWritePart::setReadWrite( rw );
}

KAboutData* NotepadPart::createAboutData()
{
  return new KAboutData( "notepadpart", I18N_NOOP( "Notepad" ), "2.0" );
}

bool NotepadPart::openFile()
{
  kdDebug() << "NotepadPart: opening " << m_file << endl;
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

  emit setStatusBarText( m_url.prettyURL() );

  return true;
}

bool NotepadPart::saveFile()
{
  if ( !isReadWrite() )
    return false;
  QFile f(m_file);
  QString s;
  if ( f.open(IO_WriteOnly) ) {
    QTextStream t( &f );
    t << m_edit->text();
    f.close();
    return true;
  } else
    return false;
}

void NotepadPart::slotSearchReplace()
{
  // What's this ? (David)
/*
  QValueList<KParts::XMLGUIServant *> plugins = KParts::Plugin::pluginServants( this );
  QValueList<KParts::XMLGUIServant *>::ConstIterator it = plugins.begin();
  QValueList<KParts::XMLGUIServant *>::ConstIterator end = plugins.end();
  for (; it != end; ++it )
    factory()->removeServant( *it );
*/
}

#include "notepad.moc"
