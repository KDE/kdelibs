#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kpluginloader.h>

#include <QtGui/QWidget>
#include <QtCore/QDir>
#include <QtCore/QFile>

#include "ghostview.h"
#include <kmimetypetrader.h>
#include <kicon.h>

Shell::Shell()
{
    // KXMLGUIClient looks in the "data" resource for the .rc files
    // This line is for test programs only!
    KGlobal::dirs()->addResourceDir( "data", KDESRCDIR );
    setXMLFile( "ghostviewtest_shell.rc" );

    KAction * paOpen = new KAction( KIcon("document-open"), "&Open file", this );
    actionCollection()->addAction( "file_open", paOpen );
    connect( paOpen, SIGNAL(triggered()), this, SLOT(slotFileOpen()) );

    KAction * paQuit = new KAction( KIcon("application-exit"), "&Quit", this );
    actionCollection()->addAction( "file_quit", paQuit );
    connect(paQuit, SIGNAL(triggered()), this, SLOT(close()));

    // Try to find a postscript component first
    m_gvpart = KMimeTypeTrader::self()->createPartInstanceFromQuery<KParts::ReadOnlyPart>( "application/postscript",
                                                                                           this,
                                                                                           this );

    // if we couldn't find a component with the trader, try the
    // kghostview library directly.  if this ever happens, then something
    // is seriously screwed up, though -- the kghostview component
    // should be picked up by the trader
    if (!m_gvpart)
    {
        KPluginLoader loader("libkghostview");
        KPluginFactory* factory = loader.factory();
        if (factory)
        {
            // Create the part
            m_gvpart = factory->create<KParts::ReadOnlyPart>( this );
        }
        else
        {
            KMessageBox::error(this, "No libkghostview found !");
        }
    }

    if ( m_gvpart )
    {
        setCentralWidget( m_gvpart->widget() );
        // Integrate its GUI
        createGUI( m_gvpart );
    }

    // Set a reasonable size
    resize( 600, 350 );
}

Shell::~Shell()
{
    delete m_gvpart;
}

void Shell::openUrl( const KUrl & url )
{
    m_gvpart->openUrl( url );
}

void Shell::slotFileOpen()
{
    KUrl url = KFileDialog::getOpenUrl( QString(), "*.ps|Postscript files (*.ps)", 0L, "file dialog" );

    if( !url.isEmpty() )
        openUrl( url );
}

int main( int argc, char **argv )
{
    KCmdLineOptions options;
    options.add("+file(s)", ki18n("Files to load"));

    const char version[] = "v0.0.1 2000 (c) David Faure";
    KLocalizedString description = ki18n("This is a test shell for the kghostview part.");

    KCmdLineArgs::init(argc, argv, "ghostviewtest", 0, ki18n("ghostviewtest"), version, description);
    KCmdLineArgs::addCmdLineOptions( options ); // Add my own options.
    KApplication app;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    Shell *shell = new Shell;
    if ( args->count() == 1 )
    {
        // Allow full paths, but also simple filenames from current dir
        KUrl url( QDir::currentPath()+"/", args->arg(0) );
        shell->openUrl( url );
    }
    shell->show();
    return app.exec();
}

#include "ghostview.moc"
