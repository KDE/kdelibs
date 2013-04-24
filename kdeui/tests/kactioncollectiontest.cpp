#include <QtTestWidgets>
#include "kactioncollectiontest.h"
#include <klocalizedstring.h>
#include <QAction>
#include <QtCore/QPointer>

#include <ksharedconfig.h>

#include <assert.h>
#include <kaboutdata.h>

void tst_KActionCollection::init()
{
    collection = new KActionCollection(static_cast<QObject *>(0));
}

void tst_KActionCollection::cleanup()
{
    delete collection;
    collection = 0;
}

void tst_KActionCollection::clear()
{
    QPointer<QAction> action1 = collection->add<QAction>("test1");
    QPointer<QAction> action2 = collection->add<QAction>("test2");
    QPointer<QAction> action3 = collection->add<QAction>("test3");
    QPointer<QAction> action4 = collection->add<QAction>("test4");
    QPointer<QAction> action5 = collection->add<QAction>("test5");
    QPointer<QAction> action6 = collection->add<QAction>("test6");
    QPointer<QAction> action7 = collection->add<QAction>("test7");

    collection->clear();
    QVERIFY(collection->isEmpty());

    QVERIFY( action1.isNull() );
    QVERIFY( action2.isNull() );
    QVERIFY( action3.isNull() );
    QVERIFY( action4.isNull() );
    QVERIFY( action5.isNull() );
    QVERIFY( action6.isNull() );
    QVERIFY( action7.isNull() );
}

void tst_KActionCollection::deleted()
{
    // Delete action -> automatically removed from collection
    QAction *a = collection->add<QAction>("test");
    delete a;
    QVERIFY(collection->isEmpty());

    // Delete action's parent -> automatically removed from collection
    QWidget* myWidget = new QWidget(0);
    QPointer<QAction> action = new QAction( /*i18n()*/ "Foo", myWidget);
    collection->addAction("foo", action);
    delete myWidget;
    QVERIFY(collection->isEmpty());
    QVERIFY(action.isNull());

    // Delete action's parent, but the action was added to another widget with setAssociatedWidget
    // and that widget gets deleted first.
    myWidget = new QWidget(0);
    QWidget* myAssociatedWidget = new QWidget(myWidget); // child widget
    action = new QAction( /*i18n()*/ "Foo", myWidget); // child action
    collection->addAction("foo", action);
    collection->addAssociatedWidget(myAssociatedWidget);
    QVERIFY(myAssociatedWidget->actions().contains(action));
    delete myAssociatedWidget; // would be done by the line below, but let's make sure it happens first
    delete myWidget;
    QVERIFY(collection->isEmpty());
    QVERIFY(action.isNull());
}

void tst_KActionCollection::take()
{
    QAction *a = collection->add<QAction>("test");
    collection->takeAction(a);
    QVERIFY(collection->isEmpty());
    delete a;
}

void tst_KActionCollection::writeSettings()
{
    KConfigGroup cfg = clearConfig();

    QList<QKeySequence> defaultShortcut;
    defaultShortcut << Qt::Key_A << Qt::Key_B;

    QList<QKeySequence> temporaryShortcut;
    temporaryShortcut << Qt::Key_C <<Qt::Key_D;

    QAction *actionWithDifferentShortcut = new QAction(this);
    collection->setDefaultShortcuts(actionWithDifferentShortcut, defaultShortcut);
    actionWithDifferentShortcut->setShortcuts(temporaryShortcut);
    collection->addAction("actionWithDifferentShortcut", actionWithDifferentShortcut);

    QAction *immutableAction = new QAction(this);
    collection->setDefaultShortcuts(immutableAction, defaultShortcut);
    immutableAction->setShortcuts(temporaryShortcut);
    collection->setShortcutsConfigurable(immutableAction, false);
    collection->addAction("immutableAction", immutableAction);

    QAction *actionWithSameShortcut = new QAction(this);
    collection->setDefaultShortcuts(actionWithSameShortcut, defaultShortcut);
    actionWithSameShortcut->setShortcuts(defaultShortcut);
    collection->addAction("actionWithSameShortcut", actionWithSameShortcut);

    cfg.writeEntry("actionToDelete", QString("Foobar"));
    QAction *actionToDelete = new QAction(this);
    collection->setDefaultShortcuts(actionToDelete, defaultShortcut);
    actionToDelete->setShortcuts(defaultShortcut);
    collection->addAction("actionToDelete", actionToDelete);

    collection->writeSettings(&cfg);

    QCOMPARE(cfg.readEntry("actionWithDifferentShortcut", QString()), QKeySequence::listToString(actionWithDifferentShortcut->shortcuts()));
    QCOMPARE(cfg.readEntry("immutableAction", QString()), QString());
    QCOMPARE(cfg.readEntry("actionWithSameShortcut", QString()), QString());
    QCOMPARE(cfg.readEntry("actionToDelete", QString()), QString());

    qDeleteAll(collection->actions());
}

void tst_KActionCollection::readSettings()
{
    KConfigGroup cfg = clearConfig();

    QList<QKeySequence> defaultShortcut;
    defaultShortcut << Qt::Key_A << Qt::Key_B;

    QList<QKeySequence> temporaryShortcut;
    temporaryShortcut << Qt::Key_C <<Qt::Key_D;

    cfg.writeEntry("normalAction", QKeySequence::listToString(defaultShortcut));
    cfg.writeEntry("immutable", QKeySequence::listToString(defaultShortcut));
    cfg.writeEntry("empty", QString());

    QAction *normal = new QAction(this);
    collection->addAction("normalAction", normal);

    QAction *immutable = new QAction(this);
    immutable->setShortcuts(temporaryShortcut);
    collection->setDefaultShortcuts(immutable, temporaryShortcut);
    collection->setShortcutsConfigurable(immutable, false);
    collection->addAction("immutable", immutable);

    QAction *empty = new QAction(this);
    collection->addAction("empty", empty);
    empty->setShortcuts(temporaryShortcut);
    collection->setDefaultShortcuts(empty, defaultShortcut);
    QCOMPARE(QKeySequence::listToString(empty->shortcuts()), QKeySequence::listToString(temporaryShortcut));

    collection->readSettings(&cfg);

    QCOMPARE(QKeySequence::listToString(normal->shortcuts()), QKeySequence::listToString(defaultShortcut));
    QCOMPARE(QKeySequence::listToString(empty->shortcuts()), QKeySequence::listToString(defaultShortcut));

    QCOMPARE(QKeySequence::listToString(immutable->shortcuts()), QKeySequence::listToString(temporaryShortcut));

    qDeleteAll(collection->actions());
}

void tst_KActionCollection::insertReplaces1()
{
    QAction *a = new QAction(0);
    QAction *b = new QAction(0);

    collection->addAction("a", a);
    QVERIFY(collection->actions().contains(a));
    QVERIFY(collection->action("a") == a);

    collection->addAction("a", b);
    QVERIFY(!collection->actions().contains(a));
    QVERIFY(collection->actions().contains(b));
    QVERIFY(collection->action("a") == b);

    delete a;
    delete b;
}

/**
 * Check that a action added twice under different names only ends up once in
 * the collection
 */
void tst_KActionCollection::insertReplaces2()
{
    QAction *a = new QAction(0);

    collection->addAction("a", a);
    QVERIFY(collection->actions().contains(a));
    QVERIFY(collection->action("a") == a);

    // Simple test: Just add it twice
    collection->addAction("b", a);
    QVERIFY(collection->actions().contains(a));
    QVERIFY(!collection->action("a"));
    QVERIFY(collection->action("b") == a);

    // Complex text: Mesh with the objectname
    a->setObjectName("c");
    collection->addAction("d", a);
    QVERIFY(collection->actions().contains(a));
    QVERIFY(!collection->action("b"));
    QVERIFY(!collection->action("c"));
    QVERIFY(collection->action("d") == a);

    delete a;
}

KConfigGroup tst_KActionCollection::clearConfig()
{
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig();
    cfg->deleteGroup(collection->configGroup());
    return KConfigGroup(cfg, collection->configGroup());
}

void tst_KActionCollection::testSetShortcuts()
{
    QAction *action = new QAction(i18n("Next Unread &Folder"), this);
    collection->addAction("go_next_unread_folder", action);
    action->setShortcut(QKeySequence(Qt::ALT+Qt::Key_Plus));
    collection->setDefaultShortcut(action, QKeySequence(Qt::ALT+Qt::Key_Plus));
    QList<QKeySequence> shortcut = action->shortcuts();
    shortcut << QKeySequence( Qt::CTRL+Qt::Key_Plus );
    action->setShortcuts( shortcut );
    QCOMPARE(QKeySequence::listToString(action->shortcuts()), QString("Alt++; Ctrl++"));

    // Simpler way:
    QList<QKeySequence> shortcut2;
    shortcut2 << QKeySequence( Qt::ALT+Qt::Key_Plus ) << QKeySequence( Qt::CTRL+Qt::Key_Plus );
    QCOMPARE(QKeySequence::listToString(shortcut2), QString("Alt++; Ctrl++"));
}

QTEST_MAIN(tst_KActionCollection)


/* vim: et sw=4 ts=4
 */
