
#include <kapplication.h>
#include <kmainwindow.h>
#include <kxmlguifactory.h>
#include <kxmlguiclient.h>
#include <kxmlguibuilder.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kstandarddirs.h>

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

    (void)new KAction( "Split", "view_left_right", 0, 0, 0, shell->actionCollection(), "splitviewh" );

    shell->setXMLFile( "./kxmlguitest_shell.rc" );

    factory->addClient( shell );

    Client *part = new Client;

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
