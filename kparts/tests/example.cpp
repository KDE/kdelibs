
#include "example.h"
#include "notepad.h"

#include <qsplitter.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qmultilinedit.h>
#include <qlineedit.h>
#include <qvbox.h>

#include <kiconloader.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <klocale.h>

Shell::Shell()
{
  setXMLFile( "kpartstest_shell.rc" );

  m_manager = new KParts::PartManager( this );

  // When the manager says the active part changes, the builder updates (recreates) the GUI
  connect( m_manager, SIGNAL( activePartChanged( Part * ) ),
           this, SLOT( createGUI( Part * ) ) );

  // We can do this "switch active part" because we have a splitter with
  // two items in it.
  // I wonder what kdevelop uses/will use to embed kedit, BTW.
  m_splitter = new QSplitter( this );

  m_part1 = new Part1(m_splitter);
  m_part2 = new Part2(m_splitter);

  QActionCollection *coll = actionCollection();

  (void)new KAction( i18n( "&View local file" ), 0, this, SLOT( slotFileOpen() ), coll, "open_local_file" );
  (void)new KAction( i18n( "&View remote file" ), 0, this, SLOT( slotFileOpenRemote() ), coll, "open_remote_file" );

  m_paEditFile = new KAction( i18n( "&Edit file" ), 0, this, SLOT( slotFileEdit() ), coll, "edit_file" );
  m_paCloseEditor = new KAction( i18n( "&Close file editor" ), 0, this, SLOT( slotFileCloseEditor() ), coll, "close_editor" );
  m_paCloseEditor->setEnabled(false);
  KAction * paQuit = new KAction( i18n( "&Quit" ), 0, this, SLOT( close() ), coll, "shell_quit" );
  paQuit->setIconSet(QIconSet(BarIcon("exit")));

  (void)new KAction( i18n( "Yet another menu item" ), 0, coll, "shell_yami" );
  (void)new KAction( i18n( "Yet another submenu item" ), 0, coll, "shell_yasmi" );

  setView( m_splitter );

  m_splitter->show();

  m_manager->addPart( m_part1 );
  m_manager->addPart( m_part2 ); // sets part 2 as the active part
  m_editorpart = 0;
}

Shell::~Shell()
{
  disconnect( m_manager, 0, this, 0 );
}

void Shell::slotFileOpen()
{
  if ( ! m_part1->openURL( locate("data", KGlobal::instance()->instanceName()+"/example_shell.rc" ) ) )
    KMessageBox::error(this,"Couldn't open file !");
}

void Shell::slotFileOpenRemote()
{
  KURL u ( "http://www.kde.org/index.html" );
  if ( ! m_part1->openURL( u ) )
    KMessageBox::error(this,"Couldn't open file !");
}

void Shell::embedEditor()
{
  if ( m_manager->activePart() == m_part2 )
    createGUI( 0L );

  // replace part2 with the editor part
  delete m_part2;
  m_part2 = 0L;
  m_editorpart = new NotepadPart( m_splitter );
  m_manager->addPart( m_editorpart );
  m_paEditFile->setEnabled(false);
  m_paCloseEditor->setEnabled(true);
}

void Shell::slotFileCloseEditor()
{
  if ( m_manager->activePart() == m_editorpart )
    createGUI( 0L );

  delete m_editorpart;
  m_editorpart = 0L;
  m_part2 = new Part2(m_splitter);
  m_manager->addPart( m_part2 );
  m_paEditFile->setEnabled(true);
  m_paCloseEditor->setEnabled(false);
}

void Shell::slotFileEdit()
{
  if ( !m_editorpart )
    embedEditor();
  // TODO use KFileDialog to allow testing remote files
  if ( ! m_editorpart->openURL( QDir::current().absPath()+"/example_shell.rc" ) )
    KMessageBox::error(this,"Couldn't open file !");
}

Part1::Part1( QWidget * parentWidget )
 : KParts::ReadOnlyPart( "Part1" )
{
  m_instance = new KInstance( "kpartstestpart" );
  setInstance( m_instance );
  m_edit = new QMultiLineEdit( parentWidget );
  setWidget( m_edit );
  setXMLFile( "kpartstest_part1.rc" );

  (void)new KAction( i18n( "Blah" ), 0, actionCollection(), "p1_blah" );
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

  manager()->setWindowCaption( m_url.url() );
  //See comments
  //manager()->statusBar()->message( m_url.url() );

  return true;
}

Part2::Part2( QWidget * parentWidget )
 : KParts::Part( "Part2" )
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

int main( int argc, char **argv )
{
  KApplication app( argc, argv, "kpartstest" );

  Shell *shell = new Shell;

  shell->show();

  app.exec();

  return 0;
}

#include "example.moc"
