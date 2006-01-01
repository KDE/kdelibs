#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kaction.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <klibloader.h>

#include <qwidget.h>
#include <qdir.h>
#include <qfile.h>

#include <ktrader.h>

#include "ghostview.h"

Shell::Shell()
{
  setXMLFile( "ghostviewtest_shell.rc" );

  KAction * paOpen = new KAction( "&Open file" , "fileopen", 0, this,
    SLOT( slotFileOpen() ), actionCollection(), "file_open" );

  KAction * paQuit = new KAction( "&Quit" , "exit", 0, this, SLOT( close() ), actionCollection(), "file_quit" );

  // Try to find a postscript component first
  KTrader::OfferList offers = KTrader::self()->query("application/postscript", "('KParts/ReadOnlyPart' in ServiceTypes) or ('Browser/View' in ServiceTypes)");

  KLibFactory *factory = 0;
  m_gvpart = 0;
  KTrader::OfferList::Iterator it(offers.begin());
  for( ; it != offers.end(); ++it)
  {
    KService::Ptr ptr = (*it);

    factory = KLibLoader::self()->factory( QFile::encodeName(ptr->library()) );
    if (factory)
    {
      m_gvpart = static_cast<KParts::ReadOnlyPart *>(factory->create(this, ptr->name().latin1(), "KParts::ReadOnlyPart"));
      setCentralWidget( m_gvpart->widget() );
      // Integrate its GUI
      createGUI( m_gvpart );

      break;
    }
  }

  // if we couldn't find a component with the trader, try the
  // kghostview library directly.  if this ever happens, then something
  // is seriously screwed up, though -- the kghostview component
  // should be picked up by the trader
  if (!m_gvpart)
  {
    factory = KLibLoader::self()->factory( "libkghostview" );
    if (factory)
    {
      // Create the part
      m_gvpart = (KParts::ReadOnlyPart *)factory->create( this, "kgvpart",
                 "KParts::ReadOnlyPart" );
      // Set the main widget
      setCentralWidget( m_gvpart->widget() );
      // Integrate its GUI
      createGUI( m_gvpart );
    }
    else
    {
       KMessageBox::error(this, "No libkghostview found !");
    }
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
  KURL url = KFileDialog::getOpenURL( QString(), "*.ps|Postscript files (*.ps)", 0L, "file dialog" );

  if( !url.isEmpty() )
     openURL( url );
}

static KCmdLineOptions options[] =
{
 { "+file(s)",          "Files to load", 0 },
 KCmdLineLastOption
};
static const char version[] = "v0.0.1 2000 (c) David Faure";
static const char description[] = "This is a test shell for the kghostview part.";

int main( int argc, char **argv )
{
  KCmdLineArgs::init(argc, argv, "ghostviewtest","ghostviewtest", description, version);
  KCmdLineArgs::addCmdLineOptions( options ); // Add my own options.
  KApplication app;
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  Shell *shell = new Shell;
  if ( args->count() == 1 )
  {
    // Allow full paths, but also simple filenames from current dir
    KURL url( QDir::currentPath()+"/", args->arg(0) );
    shell->openURL( url );
  }
  shell->show();
  return app.exec();
}

#include "ghostview.moc"
