
#include "example.h"
#include "notepad.h"
#include <kguibuilder.cpp>

#include <qsplitter.h>
#include <qlayout.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qmultilinedit.h>
#include <qvbox.h>

#include <kapp.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <klocale.h>

Shell::Shell()
{
  m_builder = new KTMainWindowGUIBuilder( this );

  m_builder->setXMLFile( "example_shell.rc" );

  m_splitter = new QSplitter( this );

  m_part1 = new Part1(m_splitter);
  m_part2 = new Part2(m_splitter);

  QActionCollection *coll = m_builder->actionCollection();

  (void)new KAction( i18n( "&View local file" ), 0, this, SLOT( slotFileOpen() ), coll, "open_local_file" );
  (void)new KAction( i18n( "&View remote file" ), 0, this, SLOT( slotFileOpenRemote() ), coll, "open_remote_file" );

  (void)new KAction( i18n( "&Edit file" ), 0, this, SLOT( slotFileEdit() ), coll, "edit_file" );
  (void)new KAction( i18n( "&Close file editor" ), 0, this, SLOT( slotFileCloseEditor() ), coll, "close_editor" );
  (void)new KAction( i18n( "&Quit" ), 0, this, SLOT( close() ), coll, "shell_quit" );

  (void)new KAction( i18n( "Yet another menu item" ), 0, coll, "shell_yami" );
  (void)new KAction( i18n( "Yet another submenu item" ), 0, coll, "shell_yasmi" );

  m_manager = new KPartManager( m_builder );

  setView( m_splitter );

  m_splitter->show();
  
  m_manager->addPart( m_part1 );
  m_manager->addPart( m_part2 );
  m_editorpart = 0;

  // Create initial GUI, setting part1 active
  m_builder->createGUI( m_part1 );
}

Shell::~Shell()
{
  disconnect( m_manager, 0, this, 0 );
}

void Shell::slotFileOpen()
{
  if ( ! m_part1->openURL( QDir::current().absPath()+"/example_shell.rc" ) )
    KMessageBox::error(this,"Couldn't open file !");
}

void Shell::slotFileOpenRemote()
{
  if ( ! m_part1->openURL( "http://www.kde.org/index.html" ) )
    KMessageBox::error(this,"Couldn't open file !");
}

void Shell::embedEditor()
{
  // replace part2 with the editor part
  delete m_part2;
  m_part2 = 0L;
  m_editorpart = new NotepadPart( m_splitter );
  m_manager->addPart( m_editorpart );
// TODO : set the active part to m_editorpart
}

void Shell::slotFileCloseEditor()
{
  delete m_editorpart;
  m_editorpart = 0L;
  m_part2 = new Part2(m_splitter);
  m_manager->addPart( m_part2 );
// Same as above
}

void Shell::slotFileEdit()
{
  if ( !m_editorpart )
    embedEditor();
  // TODO use KFileDialog to allow testing remote files
  if ( ! m_editorpart->openURL( QDir::current().absPath()+"/example_shell.rc" ) )
    KMessageBox::error(this,"Couldn't open file !");
}

void Shell::resizeEvent( QResizeEvent * )
{
  m_splitter->setGeometry( 0, 0, width(), height() );
}

Part1::Part1( QWidget * parentWidget )
 : KReadOnlyPart( "Part1" )
{
  //  m_box = new QVBox;
  //QBoxLayout *box = new QBoxLayout( this, QBoxLayout::TopToBottom );
  m_edit = new QMultiLineEdit( parentWidget );
  setWidget( m_edit );
  //box->addWidget( m_edit );

  (void)new KAction( i18n( "Blah" ), 0, actionCollection(), "p1_blah" );
}

Part1::~Part1()
{
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
  }
  m_edit->setText(s);

  if ( m_host )
  {
    m_host->setWindowCaption( m_url.url() );
    
    ((KStatusBar *)m_host->topLevelContainer( "StatusBar" ))->message( m_url.url() );
  }

  return true;
}

QString Part1::configFile() const
{
  // Hmm...  Idea : KPart::setXMLFile(), for consistency with
  // KTMainWindowGUIBuilder, and called by Part1's constructor
  // (and every other part's constructor, of course)
  // + Add a check that it's not null when using it (to prevent programming
  // errors)
  return KXMLGUIFactory::readConfigFile( "part1.rc" );
}

Part2::Part2( QWidget * parentWidget )
 : KPart( "Part2" )
{
  setWidget( new QWidget( parentWidget, "Part2Widget" ) );
}

Part2::~Part2()
{
}

QString Part2::configFile() const
{
  return QString::null;
}

int main( int argc, char **argv )
{
  KApplication app( argc, argv, "example" );
  
  Shell *shell = new Shell;
  
  shell->show();
  
  app.exec();
  
  return 0;
}

#include "example.moc"
