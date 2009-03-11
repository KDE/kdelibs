/* This file is part of the KDE libraries
    Copyright 2006 Simon Hausmann <hausmann@kde.org>
    Copyright 2008 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qtest_kde.h"
#include "kshortcut.h"

class tst_KShortcut : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void isEmpty()
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

    void checkQKeySequence()
    {
        // Check that the valid keycode Qt::Key_unknown is handled gracefully
        //
        // Qt::Key_unknown is a valid Qt Key. But toString makes unicode gibberish
        // from it. '???'.
        // Reported with patch - mjansen
        QKeySequence unknown_key(Qt::Key_unknown);
        // The keycode falls into the unicode handling
        QString p = QChar((Qt::Key_unknown-0x10000)/0x400+0xd800);
                p += QChar((Qt::Key_unknown-0x10000)%400+0xdc00);
        QCOMPARE(unknown_key.toString(), p); // What happens
        QEXPECT_FAIL("", "Qt::Key_unknown not handled", Continue);
        QCOMPARE(unknown_key.toString(), QString());     // What i would expect

        // Check that the keycode -1 is handled gracefully.
        //
        // -1 happens for some keys when listening to keyPressEvent in QWidget::event()
        // It means the key is not supported by Qt. It probably should be
        // Qt::Key_unknown instead. Unsupported keys: (Alt+Print for example).
        // Reported with patch - mjansen
        QKeySequence invalid_key(-1);
        // The keycode falls into the handling of keys lesser that Key_escape
        QString p1 = QChar((-1) & 0xffff).toUpper();
        QCOMPARE(invalid_key.toString(), QString("Meta+Ctrl+Alt+Shift+"+p1)); // What happens
        QEXPECT_FAIL("", "-1 not handled", Continue);
        QCOMPARE(invalid_key.toString(), QString());     // What i would expect

        // The famous "KDE4 eats my E key" bug: Win+E isn't parsed anymore.
        QKeySequence seq("Win+E");
        QEXPECT_FAIL("", "Qt Bug 205255/134941 - QKeySequence silently discards unknown key modifiers", Continue);
        QVERIFY(seq.isEmpty());
        // And what really happens
        QCOMPARE(seq.toString(), QLatin1String("E"));

        // KDE3 -> KDE4 migration. KDE3 used xKeycodeToKeysym or something and
        // stored the result
        QKeySequence seq2("Meta+Alt+Period");
        QEXPECT_FAIL("", "Qt Bug 205255/134941 - QKeySequence silently discards unknown key modifiers", Continue);
        QVERIFY(seq2.isEmpty());
        // And what really happens
        QCOMPARE(seq2.toString(), QLatin1String("Meta+Alt+"));
    }

    void parsing()
    {
        KShortcut cut;
        cut = KShortcut(";, Alt+;; ;, Alt+A, ;");
        QVERIFY(cut.primary() == QKeySequence::fromString(";, Alt+;"));
        QVERIFY(cut.alternate() == QKeySequence::fromString(";, Alt+A, ;"));

        cut = KShortcut("Win+E");
        //QTest::ignoreMessage(QtWarningMsg, "QKeySequence::fromString: Unknown modifier 'win+'");
        QEXPECT_FAIL("", "Qt Bug 205255 - QKeySequence silently discards unknown key modifiers", Continue);
        QVERIFY(cut.isEmpty());

        cut = KShortcut("Meta+E");
        QVERIFY(cut.primary()[0] == (Qt::META | Qt::Key_E));

        //qDebug() << QKeySequence(Qt::ALT | Qt::Key_Plus).toString();
        //qDebug() << QKeySequence(Qt::ALT | Qt::Key_Minus).toString();
        cut = KShortcut("Alt+Plus"); // KDE3 said "Alt+Plus", while Qt4 says "Alt++", so KShortcut has to handle this
        QVERIFY(cut.primary()[0] == (Qt::ALT | Qt::Key_Plus));
        cut = KShortcut("Alt+Minus"); // KDE3 said "Alt+Minus", while Qt4 says "Alt+-", so KShortcut has to handle this
        QVERIFY(cut.primary()[0] == (Qt::ALT | Qt::Key_Minus));
    }
};

QTEST_KDEMAIN(tst_KShortcut, NoGUI)

#include "kshortcuttest.moc"
