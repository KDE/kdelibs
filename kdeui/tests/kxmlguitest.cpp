#include "kxmlguitest.h"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kmainwindow.h>
#include <kxmlguifactory.h>
#include <kxmlguiclient.h>
#include <kxmlguibuilder.h>
#include <kaction.h>
#include <kdebug.h>
#include <kstdaction.h>
#include <kstandarddirs.h>
#include <qlineedit.h>
#include <qdir.h>

void Client::slotSec()
{
    kDebug() << "Client::slotSec()" << endl;
}

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "test", "Test" ,"test app" ,"1.0" );
    KApplication app;
    KAction *a;

    // KXMLGUIClient looks in the "data" resource for the .rc files
    // Let's add $PWD (ideally $srcdir instead...) to it
    KGlobal::dirs()->addResourceDir( "data", QDir::currentPath() );

    KMainWindow *mainwindow = new KMainWindow;

    QLineEdit* line = new QLineEdit( mainwindow );
    mainwindow->setCentralWidget( line );

    mainwindow->show();

    KXMLGUIBuilder *builder = new KXMLGUIBuilder( mainwindow );

    KXMLGUIFactory *factory = new KXMLGUIFactory( builder );

    Client *shell = new Client;
    shell->setInstance( new KInstance( "konqueror" ) );
    shell->instance()->dirs()->addResourceDir( "data", QDir::currentPath() );

    a = new KAction( KIcon( "view_left_right" ), "Split", shell->actionCollection(), "splitviewh" );

    shell->setXMLFile( "./kxmlguitest_shell.rc" );

    factory->addClient( shell );

    Client *part = new Client;

    a = new KAction( KIcon( "viewmag-" ), "decfont", part->actionCollection(), "decFontSizes" );
    a = new KAction( KIcon( "unlock" ), "sec", part->actionCollection(), "security" );
    a->setDefaultShortcut( Qt::ALT + Qt::Key_1 );
    a->connect( a, SIGNAL(triggered(bool)), part, SLOT( slotSec() ) );

    part->setXMLFile( "./kxmlguitest_part.rc" );

    factory->addClient( part );
    for ( int i = 0; i < 10; ++i )
    {
        factory->removeClient( part );
        factory->addClient( part );
    }

    return app.exec();
}
#include "kxmlguitest.moc"
