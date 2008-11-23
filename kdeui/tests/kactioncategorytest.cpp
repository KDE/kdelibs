#include "kactioncategorytest.h"

#include "kaction.h"
#include "kactioncollection.h"
#include "kactioncategory.h"
#include "kselectaction.h"
#include "kstandardaction.h"


void tst_KActionCategory::tstCreation()
    {
    KActionCollection collection((QObject *)NULL);
    KActionCategory category1("category1", &collection);
    KActionCategory category2("category2", &collection);

    // Check that the name is correct
    Q_ASSERT( category1.text() == "category1" );
    Q_ASSERT( category2.text() == "category2" );

    // Check that the parent is correct
    Q_ASSERT( category1.collection() == &collection );
    Q_ASSERT( category2.collection() == &collection );

    // Check that the category is available as a child of the collection
    QList<KActionCategory*> categories = collection.findChildren<KActionCategory*>();
    Q_ASSERT( categories.size() == 2);
    Q_ASSERT( categories.count(&category1) == 1 );
    Q_ASSERT( categories.count(&category2) == 1 );

    // Change the text
    category1.setText("Other Text");
    Q_ASSERT( category1.text() == "Other Text" );
    }


void tst_KActionCategory::tstSynchronization()
    {
    KActionCollection collection((QObject *)NULL);
    KActionCategory category1("category1", &collection);
    KActionCategory category2("category2", &collection);

    // The collection is empty
    Q_ASSERT( collection.count() == 0 );

    // Now add a action to category1
    KAction *action1 = category1.addAction("action1");
    // Check it was added to the category.
    Q_ASSERT( category1.actions().count() == 1 );
    Q_ASSERT( category1.actions().count(action1) == 1 );
    // Check it was added to the collection
    Q_ASSERT( collection.actions().count() == 1 );
    Q_ASSERT( collection.actions().count(action1) == 1 );

    // Short intermezzo. Add the action a second time
    category1.addAction("action1_new", action1);
    Q_ASSERT( category1.actions().count() == 1 );
    Q_ASSERT( category1.actions().count(action1) == 1 );
    Q_ASSERT( collection.actions().count() == 1 );
    Q_ASSERT( collection.actions().count(action1) == 1 );

    // Now add a action to category2
    KAction *action2 = category2.addAction("action2");
    // Check it was added to the category.
    Q_ASSERT( category2.actions().count() == 1 );
    Q_ASSERT( category2.actions().count(action2) == 1 );
    // Check it was added to the collection
    Q_ASSERT( collection.actions().count() == 2 );
    Q_ASSERT( collection.actions().count(action2) == 1 );

    // Delete action1
    delete action1;
    // Check it was removed from the collection
    Q_ASSERT( collection.actions().count() == 1 );
    Q_ASSERT( collection.actions().count(action1) == 0 );
    // Check it was removed from the category.
    Q_ASSERT( category1.actions().count() == 0 );
    Q_ASSERT( category1.actions().count(action1) == 0 );

    // Remove action2 from the collection
    collection.removeAction(action2);
    // Check it was removed from the collection
    Q_ASSERT( collection.actions().count() == 0 );
    Q_ASSERT( collection.actions().count(action2) == 0 );
    // Check it was removed from the category.
    Q_ASSERT( category2.actions().count() == 0 );
    Q_ASSERT( category2.actions().count(action2) == 0 );

    // Create another category, add a action, delete the category and check
    // if the action is still part of the collection.
    KActionCategory *category3 = new KActionCategory("category3", &collection);
    KAction *action3 = category3->addAction( "action3" );
    // Check it was added to the collection
    Q_ASSERT( collection.actions().count(action3) == 1 );
    // delete the category
    delete category3;
    // Make sure the action is still there.
    Q_ASSERT( collection.actions().count(action3) == 1 );
    }


void tst_KActionCategory::tstActionCreation()
    {
    KActionCollection collection((QObject *)NULL);
    KActionCategory category("category", &collection);

    // QAction * addAction(const QString &name, QAction *action);
    QAction *action1 = new KAction(NULL);
    category.addAction("action1", action1);
    Q_ASSERT( category.actions().count(action1) == 1 );
    Q_ASSERT( collection.actions().count(action1) == 1 );

    // KAction * addAction(const QString &name, KAction *action);
    KAction *action2 = new KAction(NULL);
    category.addAction("action2", action2);
    Q_ASSERT( category.actions().count(action2) == 1 );
    Q_ASSERT( collection.actions().count(action2) == 1 );

    // KAction * addAction(
    //         KStandardAction::StandardAction actionType,
    //         const QObject *receiver = NULL,
    //         const char *member = NULL);
    KAction *action3 = category.addAction(KStandardAction::Revert);
    Q_ASSERT( category.actions().count(action3) == 1 );
    Q_ASSERT( collection.actions().count(action3) == 1 );

    // KAction * addAction(
    //         KStandardAction::StandardAction actionType,
    //         const QString &name,
    //         const QObject *receiver = NULL,
    //         const char *member = NULL);
    KAction *action4 = category.addAction(KStandardAction::Quit, "myownname");
    Q_ASSERT( action4->objectName() == "myownname" );
    Q_ASSERT( category.actions().count(action4) == 1 );
    Q_ASSERT( collection.actions().count(action4) == 1 );

    // KAction *addAction(
    //         const QString &name,
    //         const QObject *receiver = NULL,
    //         const char *member = NULL);
    KAction *action5 = category.addAction("action5");
    Q_ASSERT( category.actions().count(action5) == 1 );
    Q_ASSERT( collection.actions().count(action5) == 1 );

    // template<class ActionType>
    // ActionType *add(
    //         const QString &name,
    //         const QObject *receiver = NULL,
    //         const char *member = NULL)
    KSelectAction *action6 = category.add<KSelectAction>("action6");
    Q_ASSERT( category.actions().count(action6) == 1 );
    Q_ASSERT( collection.actions().count(action6) == 1 );

    // There should be 6 actions inside the collection and category
    Q_ASSERT( category.actions().count() == 6 );
    Q_ASSERT( collection.actions().count() == 6 );
    }


QTEST_KDEMAIN(tst_KActionCategory, GUI)

#include "moc_kactioncategorytest.cpp"
