
#include "example.h"
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

  m_part1 = new Part1();
  m_part2 = new Part2();

  QActionCollection *coll = m_builder->actionCollection();

  (void)new KAction( i18n( "&Open local file" ), 0, this, SLOT( slotFileOpen() ), coll, "open_local_file" );
  (void)new KAction( i18n( "&Open remote file" ), 0, this, SLOT( slotFileOpenRemote() ), coll, "open_remote_file" );
  (void)new KAction( i18n( "&Quit" ), 0, this, SLOT( close() ), coll, "shell_quit" );

  (void)new KAction( i18n( "Yet another menu item" ), 0, coll, "shell_yami" );
  (void)new KAction( i18n( "Yet another submenu item" ), 0, coll, "shell_yasmi" );

  m_manager = new KPartManager( this );

  connect( m_manager, SIGNAL( activePartChanged( KPart *, KPart * ) ),
           this, SLOT( slotActivePartChanged( KPart *, KPart * ) ) );
  
  m_splitter = new QSplitter( this );

  setView( m_splitter );

  m_splitter->show();
  
  m_part1->embed( m_splitter );
  m_part2->embed( m_splitter );

  m_manager->addPart( m_part1 );
  m_manager->addPart( m_part2 );

  slotActivePartChanged( m_part1, 0 );
}

Shell::~Shell()
{
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

void Shell::slotActivePartChanged( KPart *newPart, KPart *oldPart )
{
  qDebug( "%s -> %s", oldPart ? oldPart->name() : "0L",
                      newPart ? newPart->name() : "0L");

  m_builder->createGUI( newPart );
}

void Shell::resizeEvent( QResizeEvent * )
{
  m_splitter->setGeometry( 0, 0, width(), height() );
}

Part1::Part1()
 : KReadOnlyPart( "Part1" )
{
  //  m_box = new QVBox;
  //QBoxLayout *box = new QBoxLayout( this, QBoxLayout::TopToBottom );
  m_edit = new QMultiLineEdit( 0L );
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
  // Hmm...
  return KXMLGUIFactory::readConfigFile( "part1.rc" );
}

Part2::Part2()
 : KPart( "Part2" )
{
  setWidget( new QWidget );
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
