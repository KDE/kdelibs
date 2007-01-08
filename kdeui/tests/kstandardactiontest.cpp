#include "kstandardactiontest.h"

#include "kstandardaction.h"
#include "kactioncollection.h"
#include "kaction.h"

void tst_KStandardAction::implicitInsertion()
{
    KActionCollection collection(static_cast<QObject *>(0));
    KAction *a = KStandardAction::create(KStandardAction::Undo, this, SLOT(dummy()), &collection);
    QVERIFY(a);

    QVERIFY(a->parent() == &collection);
    QVERIFY(collection.action(KStandardAction::name(KStandardAction::Undo)) == a);
}

QTEST_KDEMAIN(tst_KStandardAction, GUI)

#include "kstandardactiontest.moc"
