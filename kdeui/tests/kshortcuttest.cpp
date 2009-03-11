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

    void parsing()
    {
        KShortcut cut;
        cut = KShortcut(";, Alt+;; ;, Alt+A, ;");
        QVERIFY(cut.primary() == QKeySequence::fromString(";, Alt+;"));
        QVERIFY(cut.alternate() == QKeySequence::fromString(";, Alt+A, ;"));

#if 0 // TODO re-enable once fixed in Qt
        // The famous "KDE4 eats my E key" bug: Win+E isn't parsed anymore.
        QKeySequence seq("Win+E");
        QTest::ignoreMessage(QtWarningMsg, "QKeySequence::fromString: Unknown modifier 'win+'");
        QVERIFY(seq.isEmpty());

        cut = KShortcut("Win+E");
        //QTest::ignoreMessage(QtWarningMsg, "QKeySequence::fromString: Unknown modifier 'win+'");
        QVERIFY(cut.isEmpty());
#endif

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
