#include "kshortcuttest.h"

void tst_KShortcut::isEmpty()
{
    KShortcut cut;
    QVERIFY(cut.isEmpty());

    cut = KShortcut(0, 0);
    QVERIFY(cut.isEmpty());

    cut = KShortcut(QKeySequence());
    QVERIFY(cut.isEmpty());

    cut = KShortcut(QKeySequence(), QKeySequence());
    QVERIFY(cut.isEmpty());

    cut = KShortcut(QList<QKeySequence>());
    QVERIFY(cut.isEmpty());

    cut = KShortcut(Qt::Key_A);
    QVERIFY(!cut.isEmpty());
}

QTEST_KDEMAIN(tst_KShortcut, NoGUI)

#include "kshortcuttest.moc"
