#include <kiconloader.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kaction.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <klibloader.h>
#include <qwidget.h>
#include <qdir.h>

#include "ghostview.h"

Shell::Shell()
{
  setXMLFile( "ghostviewtest_shell.rc" );

  KAction * paOpen = new KAction( i18n( "&Open file" ), 0, this, SLOT( slotFileOpen() ), actionCollection(), "file_open" );
  paOpen->setIconSet(QIconSet(BarIcon("fileopen")));

  KAction * paQuit = new KAction( i18n( "&Quit" ), 0, this, SLOT( close() ), actionCollection(), "file_quit" );
  paQuit->setIconSet(QIconSet(BarIcon("exit")));

  // Try to find libkghostview
  KLibFactory *factory = KLibLoader::self()->factory( "libkghostview" );
  if (factory)
  {
    // Create the part
    m_gvpart = (KParts::ReadOnlyPart *)factory->create( this, "kgvpart",
               "KParts::ReadOnlyPart" );
    // Set the main widget
    setView( m_gvpart->widget() );
    // Integrate its GUI
    createGUI( m_gvpart );
  }
  else
  {
     KMessageBox::error(this, i18n("No libkghostview found !"));
  }
  // Set a reasonable size
  resize( 600, 350 );
}

Shell::~Shell()
{
  delete m_gvpart;
}

void Shell::openURL( const KURL & url )
{
  m_gvpart->openURL( url );
}

void Shell::slotFileOpen()
{
  QString fileName = KFileDialog::getOpenFileName( QString::null, "*.ps|Postscript files (*.ps)", 0L, "file dialog" );
 
  if( !fileName.isEmpty() )
     openURL( KURL( fileName ) );
}

static KCmdLineOptions options[] =
{
 { "+file(s)",          I18N_NOOP("Files to load"), 0 },
 { 0,0,0 }
};
static const char *version = "v0.0.1 2000 (c) David Faure";
static const char *description = I18N_NOOP("This is a test shell for the kghostview part.");

int main( int argc, char **argv )
{
  KCmdLineArgs::init(argc, argv, "ghostviewtest", description, version);
  KCmdLineArgs::addCmdLineOptions( options ); // Add my own options.
  KApplication app;
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  Shell *shell = new Shell;
  if ( args->count() == 1 )
  {
    // Allow full paths, but also simple filenames from current dir
    KURL url( QDir::currentDirPath()+"/", args->arg(0) );
    shell->openURL( url );
  }
  shell->show();
  return app.exec();
}

#include "ghostview.moc"
