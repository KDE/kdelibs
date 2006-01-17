
#include "example.h"
#include "parts.h"
#include "notepad.h"

#include <qsplitter.h>
#include <qcheckbox.h>
#include <qdir.h>
#include <kcmdlineargs.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <klocale.h>

Shell::Shell()
{
  setXMLFile( "kpartstest_shell.rc" );

  m_manager = new KParts::PartManager( this );

  // When the manager says the active part changes, the builder updates (recreates) the GUI
  connect( m_manager, SIGNAL( activePartChanged( KParts::Part * ) ),
           this, SLOT( createGUI( KParts::Part * ) ) );

  // We can do this "switch active part" because we have a splitter with
  // two items in it.
  // I wonder what kdevelop uses/will use to embed kedit, BTW.
  m_splitter = new QSplitter( this );

  m_part1 = new Part1(this, m_splitter);
  m_part2 = new Part2(this, m_splitter);

  KActionCollection *coll = actionCollection();

  (void)new KAction( "&View local file", 0, this, SLOT( slotFileOpen() ), coll, "open_local_file" );
  (void)new KAction( "&View remote file", 0, this, SLOT( slotFileOpenRemote() ), coll, "open_remote_file" );

  m_paEditFile = new KAction( "&Edit file", 0, this, SLOT( slotFileEdit() ), coll, "edit_file" );
  m_paCloseEditor = new KAction( "&Close file editor", 0, this, SLOT( slotFileCloseEditor() ), coll, "close_editor" );
  m_paCloseEditor->setEnabled(false);
  KAction * paQuit = new KAction( "&Quit", 0, this, SLOT( close() ), coll, "shell_quit" );
  paQuit->setIcon(QIcon(BarIcon("exit")));

//  (void)new KAction( "Yet another menu item", 0, coll, "shell_yami" );
//  (void)new KAction( "Yet another submenu item", 0, coll, "shell_yasmi" );

  setCentralWidget( m_splitter );
  m_splitter->setMinimumSize( 400, 300 );

  m_splitter->show();

  m_manager->addPart( m_part1, true ); // sets part 1 as the active part
  m_manager->addPart( m_part2, false );
  m_editorpart = 0;
}

Shell::~Shell()
{
  disconnect( m_manager, 0, this, 0 );
}

void Shell::slotFileOpen()
{
  if ( ! m_part1->openURL( locate("data", KGlobal::instance()->instanceName()+"/kpartstest_shell.rc" ) ) )
    KMessageBox::error(this,"Couldn't open file !");
}

void Shell::slotFileOpenRemote()
{
  KUrl u ( "http://www.kde.org/index.html" );
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
  m_editorpart = new NotepadPart( m_splitter, "editor",
                                  this, "NotepadPart" );
  m_editorpart->setReadWrite(); // read-write mode
  m_manager->addPart( m_editorpart );
  m_paEditFile->setEnabled(false);
  m_paCloseEditor->setEnabled(true);
}

void Shell::slotFileCloseEditor()
{
  // It is very important to close the url of a read-write part
  // before destroying it. This allows to save the document (if modified)
  // and also to cancel.
  if ( ! m_editorpart->closeURL() )
    return;

  // Is this necessary ? (David)
  if ( m_manager->activePart() == m_editorpart )
    createGUI( 0L );

  delete m_editorpart;
  m_editorpart = 0L;
  m_part2 = new Part2(this, m_splitter);
  m_manager->addPart( m_part2 );
  m_paEditFile->setEnabled(true);
  m_paCloseEditor->setEnabled(false);
}

void Shell::slotFileEdit()
{
  if ( !m_editorpart )
    embedEditor();
  // TODO use KFileDialog to allow testing remote files
  if ( ! m_editorpart->openURL( QDir::current().absolutePath()+"/kpartstest_shell.rc" ) )
    KMessageBox::error(this,"Couldn't open file !");
}

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "kpartstest",0,0,0,0 );
  KApplication app;

  Shell *shell = new Shell;

  shell->show();

  app.exec();

  return 0;
}

#include "example.moc"
