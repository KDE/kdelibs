#include <kiconloader.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kaction.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <klibloader.h>
#include <qwidget.h>

#include "ghostview.h"

Shell::Shell()
{
  setXMLFile( "ghostviewtest_shell.rc" );

  KAction * paOpen = new KAction( i18n( "&Open file" ), 0, this, SLOT( slotFileOpen() ), actionCollection(), "file_open" );
  paOpen->setIconSet(QIconSet(BarIcon("fileopen")));

  KAction * paQuit = new KAction( i18n( "&Quit" ), 0, this, SLOT( close() ), actionCollection(), "file_quit" );
  paQuit->setIconSet(QIconSet(BarIcon("exit")));

 // Looks like KTMainWindow needs to use a dummy widget ?
  m_mainWidget = new QWidget( this );
  m_mainWidget->setMinimumSize(400,300);
  setView( m_mainWidget );
  m_gvpart = 0;

  createGUI( 0 ); // no active part
}

Shell::~Shell()
{
  delete m_gvpart;
}

void Shell::slotFileOpen()
{
  QString fileName = KFileDialog::getOpenFileName( QString::null, QString::null, 0L, "file dialog" );
 
  if( !fileName.isEmpty() )
  {
    // Try to find libkghostview
    KLibFactory *factory = KLibLoader::self()->factory( "libkghostview" );
    if (factory)
    {
      // Create the part
      m_gvpart = (KParts::ReadOnlyPart *)factory->create( this, "kgvpart",
                 "KParts::ReadOnlyPart" );
      setView( m_gvpart->widget() );
      // Integrate its GUI
      createGUI( m_gvpart );
      // No need for this anymore
      delete m_mainWidget;
      m_mainWidget = 0;
    }
  }
}

int main( int argc, char **argv )
{
  KApplication app( argc, argv, "ghostviewtest" );
  Shell *shell = new Shell;
  shell->show();
  return app.exec();
}

#include "ghostview.moc"
