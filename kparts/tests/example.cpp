
#include "example.h"

#include <qsplitter.h>

#include <kapp.h>
#include <kaction.h>
#include <klocale.h>

Shell::Shell( KPart *part1, KPart *part2 )
{
  m_part1 = part1;
  m_part2 = part2;

  (void)new KAction( i18n( "&Quit" ), 0, this, SLOT( close() ), actionCollection(), "shell_quit" );

  m_manager = new KEmbedManager( this );

  connect( m_manager, SIGNAL( activePartChanged( KPart *, KPart * ) ),
           this, SLOT( slotActivePartChanged( KPart *, KPart * ) ) );
  
  m_splitter = new QSplitter( this );

  setView( m_splitter );

  m_splitter->show();
  
  m_part1->reparent( m_splitter, 0, QPoint( 0, 0 ), true );
  m_part2->reparent( m_splitter, 0, QPoint( 0, 0 ), true );
  
  m_manager->addPart( m_part1 );
  m_manager->addPart( m_part2 );

  setActivePart( m_part1 );
}

Shell::~Shell()
{
}

void Shell::slotActivePartChanged( KPart *newPart, KPart *oldPart )
{
  if ( newPart && oldPart )
    qDebug( "%s -> %s", newPart->name(), oldPart->name() );
  if ( newPart )
    setActivePart( newPart );
}

QString Shell::config() const
{
  return readConfigFile( "example_shell.rc" );
}

void Shell::resizeEvent( QResizeEvent * )
{
  m_splitter->setGeometry( 0, 0, width(), height() );
}

Part1::Part1()
 : KReadOnlyPart( 0, "Part1" )
{
}

Part1::~Part1()
{
}

bool Part1::openFile()
{
  debug("Part1: opening %s", m_file.ascii());
  return true;
}

QString Part1::configFile() const
{
  return QString::null;
}

Part2::Part2()
 : KPart( 0, "Part2" )
{
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
  
  KReadOnlyPart *part1 = new Part1;
  KPart *part2 = new Part2;
  
  Shell *shell = new Shell( part1, part2 );
  
  shell->show();
  
  app.exec();
  
  return 0;
}

#include "example.moc"
