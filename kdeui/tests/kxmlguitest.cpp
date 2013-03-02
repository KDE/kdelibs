#include "kxmlguitest.h"
#include <QAction>
#include <QApplication>
#include <kmainwindow.h>
#include <kxmlguifactory.h>
#include <kxmlguiclient.h>
#include <kxmlguibuilder.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <QLineEdit>
#include <QtCore/QDir>
#include <QtTest/QtTest>

void Client::slotSec()
{
    kDebug() << "Client::slotSec()";
}

int main( int argc, char **argv )
{
    QApplication::setApplicationName("test");
    QApplication app(argc, argv);
    QAction *a;

    KMainWindow *mainwindow = new KMainWindow;

    QLineEdit* line = new QLineEdit( mainwindow );
    mainwindow->setCentralWidget( line );

    mainwindow->show();

    KXMLGUIBuilder *builder = new KXMLGUIBuilder( mainwindow );

    KXMLGUIFactory *factory = new KXMLGUIFactory( builder );

    Client *shell = new Client;
    shell->setComponentName("konqueror", "Konqueror");

    a = new QAction( QIcon::fromTheme( "view-split-left-right" ), "Split", shell );
    shell->actionCollection()->addAction( "splitviewh", a );

    shell->setXMLFile( QFINDTESTDATA("kxmlguitest_shell.rc") );

    factory->addClient( shell );

    Client *part = new Client;

    a = new QAction( QIcon::fromTheme( "zoom-out" ), "decfont", part );
    part->actionCollection()->addAction( "decFontSizes", a );
    a = new QAction( QIcon::fromTheme( "security-low" ), "sec", part );
    part->actionCollection()->addAction( "security", a );
    part->actionCollection()->setDefaultShortcuts(a, KShortcut(Qt::ALT + Qt::Key_1));
    a->connect( a, SIGNAL(triggered(bool)), part, SLOT(slotSec()) );

    part->setXMLFile( QFINDTESTDATA("kxmlguitest_part.rc") );

    factory->addClient( part );
    for ( int i = 0; i < 10; ++i )
    {
        factory->removeClient( part );
        factory->addClient( part );
    }

    return app.exec();
}
