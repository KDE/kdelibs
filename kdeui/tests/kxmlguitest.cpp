
#include <kapp.h>
#include <kmainwindow.h>
#include <kxmlgui.h>
#include <kxmlguiclient.h>
#include <kxmlguibuilder.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kstddirs.h>

class Client : public KXMLGUIClient
{
public:
    Client() {}

    void setXMLFile( const QString &f, bool merge = true ) { KXMLGUIClient::setXMLFile( f, merge ); }
    void setInstance( KInstance *inst ) { KXMLGUIClient::setInstance( inst ); }
};

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "kxmlguitest" );

    KMainWindow *mainwindow = new KMainWindow;
    mainwindow->show();

    KXMLGUIBuilder *builder = new KXMLGUIBuilder( mainwindow );

    KXMLGUIFactory *factory = new KXMLGUIFactory( builder );

    Client *shell = new Client;
    shell->setInstance( new KInstance( "konqueror" ) );

    KStdAction::up( 0, 0, shell->actionCollection(), "up" );
    KStdAction::back( 0, 0, shell->actionCollection(), "back" );
    KStdAction::forward( 0, 0, shell->actionCollection(), "forward" );
    KStdAction::home( 0, 0, shell->actionCollection(), "home" );
    (void)new KAction( "Reload", "reload", 0, 0, 0, shell->actionCollection(), "reload" );
    (void)new KAction( "Stop", "stop", 0, 0, 0, shell->actionCollection(), "stop" );
    KStdAction::cut( 0, 0, shell->actionCollection(), "cut" );
    KStdAction::copy( 0, 0, shell->actionCollection(), "copy" );
    KStdAction::paste( 0, 0, shell->actionCollection(), "paste" );
    KStdAction::print( 0, 0, shell->actionCollection(), "print" );

    (void)new KAction( "Split", "view_left_right", 0, 0, 0, shell->actionCollection(), "splitviewh" );
    (void)new KAction( "Logo", "editdelete", 0, 0, 0, shell->actionCollection(), "animated_logo" );

//    shell->setXMLFile( locate( "data", "config/ui_standards.rc" ) );
    shell->setXMLFile( "./kxmlguitest_shell.rc" );

    factory->addClient( shell );

    Client *part = new Client;

    (void)new KAction( "Print Frame", "frameprint", 0, 0, 0, part->actionCollection(), "printFrame" );
    KStdAction::find( 0, 0, part->actionCollection(), "find" );
    (void)new KAction( "incfont", "viewmag+", 0, 0, 0, part->actionCollection(), "incFontSizes" );
    (void)new KAction( "decfont", "viewmag-", 0, 0, 0, part->actionCollection(), "decFontSizes" );
    (void)new KAction( "sec", "unlock", 0, 0, 0, part->actionCollection(), "security" );

    part->setXMLFile( "./kxmlguitest_part.rc" );

    factory->addClient( part );
    for ( int i = 0; i < 10; ++i )
    {
        factory->removeClient( part );
        factory->addClient( part );
    }

    return app.exec();
}
