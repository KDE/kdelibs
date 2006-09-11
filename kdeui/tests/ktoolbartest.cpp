#include <kapplication.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <QMainWindow>
#include <ktoolbar.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

int main( int argc, char **argv )
{
    KAboutData aboutData( "kactiontest", "kactiontest", "1.0" );
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    KActionCollection coll( static_cast<QObject *>( 0 ) );

    KAction* action1 = new KAction("test1", &coll, "test1");
    KAction* action2 = new KAction("test2", &coll, "test2");
    KAction* action3 = new KAction("test3", &coll, "test3");
    new KAction("test4", &coll, "test4");
    new KAction("test5", &coll, "test5");
    new KAction("test6", &coll, "test6");
    KAction* action7 = new KAction("test7", &coll, "test7");

    QMainWindow* mw = new QMainWindow();
    KToolBar* tb = new KToolBar(mw);
    mw->addToolBar(tb);

    action2->setSeparator(true);
    action3->setSeparator(true);
    action7->setSeparator(true);

    coll.setAssociatedWidget(tb);

    mw->show();

    app.exec();

    mw->show();

    action2->setVisible(false);

    app.exec();

    mw->show();

    action1->setVisible(false);

    return app.exec();
}

/* vim: et sw=4 ts=4
 */
