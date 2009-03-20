
#include "kactioncollectiontest.h"
#include <QtCore/QPointer>

#include <kapplication.h>
#include <kaction.h>
#include <kglobal.h>
#include <kconfig.h>

#include <assert.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

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
    QPointer<QAction> action1 = collection->addAction("test1");
    QPointer<QAction> action2 = collection->addAction("test2");
    QPointer<QAction> action3 = collection->addAction("test3");
    QPointer<QAction> action4 = collection->addAction("test4");
    QPointer<QAction> action5 = collection->addAction("test5");
    QPointer<QAction> action6 = collection->addAction("test6");
    QPointer<QAction> action7 = collection->addAction("test7");

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
    QAction *a = collection->addAction("test");
    delete a;
    QVERIFY(collection->isEmpty());

    // Delete action's parent -> automatically removed from collection
    QWidget* myWidget = new QWidget(0);
    QPointer<KAction> action = new KAction( /*i18n()*/ "Foo", myWidget);
    collection->addAction("foo", action);
    delete myWidget;
    QVERIFY(collection->isEmpty());
    QVERIFY(action.isNull());

    // Delete action's parent, but the action was added to another widget with setAssociatedWidget
    // and that widget gets deleted first.
    myWidget = new QWidget(0);
    QWidget* myAssociatedWidget = new QWidget(myWidget); // child widget
    action = new KAction( /*i18n()*/ "Foo", myWidget); // child action
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
    QAction *a = collection->addAction("test");
    collection->takeAction(a);
    QVERIFY(collection->isEmpty());
    delete a;
}

void tst_KActionCollection::writeSettings()
{
    KConfigGroup cfg = clearConfig();

    KShortcut defaultShortcut;
    defaultShortcut.setPrimary(Qt::Key_A);
    defaultShortcut.setAlternate(Qt::Key_B);

    KShortcut temporaryShortcut;
    temporaryShortcut.setPrimary(Qt::Key_C);
    temporaryShortcut.setAlternate(Qt::Key_D);

    KAction *actionWithDifferentShortcut = new KAction(this);
    actionWithDifferentShortcut->setShortcut(defaultShortcut, KAction::DefaultShortcut);
    actionWithDifferentShortcut->setShortcut(temporaryShortcut, KAction::ActiveShortcut);
    collection->addAction("actionWithDifferentShortcut", actionWithDifferentShortcut);

    KAction *immutableAction = new KAction(this);
    immutableAction->setShortcut(defaultShortcut, KAction::DefaultShortcut);
    immutableAction->setShortcut(temporaryShortcut, KAction::ActiveShortcut);
    immutableAction->setShortcutConfigurable(false);
    collection->addAction("immutableAction", immutableAction);

    KAction *actionWithSameShortcut = new KAction(this);
    actionWithSameShortcut->setShortcut(defaultShortcut, KAction::DefaultShortcut);
    actionWithSameShortcut->setShortcut(defaultShortcut, KAction::ActiveShortcut);
    collection->addAction("actionWithSameShortcut", actionWithSameShortcut);

    cfg.writeEntry("actionToDelete", QString("Foobar"));
    KAction *actionToDelete = new KAction(this);
    actionToDelete->setShortcut(defaultShortcut, KAction::DefaultShortcut);
    actionToDelete->setShortcut(defaultShortcut, KAction::ActiveShortcut);
    collection->addAction("actionToDelete", actionToDelete);

    collection->writeSettings(&cfg);

    QCOMPARE(cfg.readEntry("actionWithDifferentShortcut", QString()), KShortcut(actionWithDifferentShortcut->shortcuts()).toString());
    QCOMPARE(cfg.readEntry("immutableAction", QString()), QString());
    QCOMPARE(cfg.readEntry("actionWithSameShortcut", QString()), QString());
    QCOMPARE(cfg.readEntry("actionToDelete", QString()), QString());

    qDeleteAll(collection->actions());
}

void tst_KActionCollection::readSettings()
{
    KConfigGroup cfg = clearConfig();

    KShortcut defaultShortcut;
    defaultShortcut.setPrimary(Qt::Key_A);
    defaultShortcut.setAlternate(Qt::Key_B);

    KShortcut temporaryShortcut;
    temporaryShortcut.setPrimary(Qt::Key_C);
    temporaryShortcut.setAlternate(Qt::Key_D);

    cfg.writeEntry("normalAction", defaultShortcut.toString());
    cfg.writeEntry("immutable", defaultShortcut.toString());
    cfg.writeEntry("empty", QString());

    KAction *normal = new KAction(this);
    collection->addAction("normalAction", normal);

    KAction *immutable = new KAction(this);
    immutable->setShortcut(temporaryShortcut, KAction::ActiveShortcut);
    immutable->setShortcut(temporaryShortcut, KAction::DefaultShortcut);
    immutable->setShortcutConfigurable(false);
    collection->addAction("immutable", immutable);

    KAction *empty = new KAction(this);
    collection->addAction("empty", empty);
    empty->setShortcut(temporaryShortcut, KAction::ActiveShortcut);
    empty->setShortcut(defaultShortcut, KAction::DefaultShortcut);
    QCOMPARE(KShortcut(empty->shortcuts()).toString(), temporaryShortcut.toString());

    collection->readSettings(&cfg);

    QCOMPARE(KShortcut(normal->shortcuts()).toString(), defaultShortcut.toString());
    QCOMPARE(KShortcut(empty->shortcuts()).toString(), defaultShortcut.toString());

    QCOMPARE(KShortcut(immutable->shortcuts()).toString(), temporaryShortcut.toString());

    qDeleteAll(collection->actions());
}

void tst_KActionCollection::insertReplaces1()
{
    KAction *a = new KAction(0);
    KAction *b = new KAction(0);

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
    KAction *a = new KAction(0);

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
    KSharedConfig::Ptr cfg = KGlobal::config();
    cfg->deleteGroup(collection->configGroup());
    return KConfigGroup(cfg, collection->configGroup());
}

void tst_KActionCollection::testSetShortcuts()
{
    KAction *action = new KAction(i18n("Next Unread &Folder"), this);
    collection->addAction("go_next_unread_folder", action);
    action->setShortcut(QKeySequence(Qt::ALT+Qt::Key_Plus));
    KShortcut shortcut = KShortcut(action->shortcuts());
    shortcut.setAlternate( QKeySequence( Qt::CTRL+Qt::Key_Plus ) );
    action->setShortcuts( shortcut );
    QCOMPARE(action->shortcut().toString(), QString("Alt++; Ctrl++"));

    // Simpler way:
    KShortcut shortcut2;
    shortcut2.setPrimary( QKeySequence( Qt::ALT+Qt::Key_Plus ) );
    shortcut2.setAlternate( QKeySequence( Qt::CTRL+Qt::Key_Plus ) );
    QCOMPARE(shortcut2.toString(), QString("Alt++; Ctrl++"));
}

QTEST_KDEMAIN(tst_KActionCollection, GUI)

#include "kactioncollectiontest.moc"

/* vim: et sw=4 ts=4
 */
