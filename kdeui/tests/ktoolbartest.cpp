#include <QApplication>
#include <QAction>
#include <kactioncollection.h>
#include <QMainWindow>
#include <ktoolbar.h>

// This is a test for "Automatically hide extra toolbar separators"
// If several separators are next to each other, only one should show up.

int main( int argc, char **argv )
{
    QApplication::setApplicationName("kactiontest");
    QApplication app(argc, argv);

    KActionCollection coll( static_cast<QObject *>( 0 ) );

    QAction* action1 = coll.addAction("test1");
    action1->setText("test1");
    QAction* action2 = coll.addAction("test2");
    action2->setText("test2");
    QAction* action3 = coll.addAction("test3");
    action3->setText("test3");
    QAction* action4 = coll.addAction("test4");
    action4->setText("test4");
    QAction* action5 = coll.addAction("test5");
    action5->setText("test5");
    QAction* action6 = coll.addAction("test6");
    action6->setText("test6");
    QAction* action7 = coll.addAction("test7");
    action7->setText("test7");

    QMainWindow* mw = new QMainWindow();
    KToolBar* tb = new KToolBar(mw);
    mw->addToolBar(tb);

    action2->setSeparator(true);
    action3->setSeparator(true);
    action7->setSeparator(true);

    coll.addAssociatedWidget(tb);

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
