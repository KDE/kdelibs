#include "kxmlguitest.h"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kmainwindow.h>
#include <kxmlguifactory.h>
#include <kxmlguiclient.h>
#include <kxmlguibuilder.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <kicon.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <QtGui/QLineEdit>
#include <QtCore/QDir>

void Client::slotSec()
{
    kDebug() << "Client::slotSec()";
}

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "test", 0, ki18n("Test"), "1.0", ki18n("test app"));
    KApplication app;
    KAction *a;

    // KXMLGUIClient looks in the "data" resource for the .rc files
    // Let's add the source dir to it
    KGlobal::dirs()->addResourceDir( "data", KDESRCDIR );

    KMainWindow *mainwindow = new KMainWindow;

    QLineEdit* line = new QLineEdit( mainwindow );
    mainwindow->setCentralWidget( line );

    mainwindow->show();

    KXMLGUIBuilder *builder = new KXMLGUIBuilder( mainwindow );

    KXMLGUIFactory *factory = new KXMLGUIFactory( builder );

    Client *shell = new Client;
    shell->setComponentData(KComponentData("konqueror"));
    shell->componentData().dirs()->addResourceDir( "data", QDir::currentPath() );

    a = new KAction( KIcon( "view-split-left-right" ), "Split", shell );
    shell->actionCollection()->addAction( "splitviewh", a );

    shell->setXMLFile( KDESRCDIR "/kxmlguitest_shell.rc" );

    factory->addClient( shell );

    Client *part = new Client;

    a = new KAction( KIcon( "zoom-out" ), "decfont", part );
    part->actionCollection()->addAction( "decFontSizes", a );
    a = new KAction( KIcon( "security-low" ), "sec", part );
    part->actionCollection()->addAction( "security", a );
    a->setShortcut( KShortcut(Qt::ALT + Qt::Key_1), KAction::DefaultShortcut );
    a->connect( a, SIGNAL(triggered(bool)), part, SLOT( slotSec() ) );

    part->setXMLFile( KDESRCDIR "/kxmlguitest_part.rc" );

    factory->addClient( part );
    for ( int i = 0; i < 10; ++i )
    {
        factory->removeClient( part );
        factory->addClient( part );
    }

    return app.exec();
}
#include "kxmlguitest.moc"
