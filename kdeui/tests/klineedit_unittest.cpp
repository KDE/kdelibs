/* This file is part of the KDE libraries

    Copyright (c) 2007 David Faure <faure@kde.org>

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

#include <QClipboard>
#include <qtest_kde.h>
#include <qtestevent.h>
#include <klineedit.h>

class KLineEdit_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testPassword()
    {
        KLineEdit w;
        w.setPasswordMode(true);
        QTest::keyClick(&w, Qt::Key_1);
        QTest::keyClick(&w, Qt::Key_2);
        QTest::keyClick(&w, Qt::Key_3);
        QCOMPARE(w.text(), QString("123"));
    }

    void testReturnPressed()
    {
        KLineEdit w;
        w.setText("Hello world");
        QSignalSpy qReturnPressedSpy(&w, SIGNAL(returnPressed()));
        QSignalSpy kReturnPressedSpy(&w, SIGNAL(returnPressed(QString)));
        QTest::keyClick(&w, Qt::Key_Return);
        QCOMPARE(qReturnPressedSpy.count(), 1);
        QCOMPARE(kReturnPressedSpy.count(), 1);
        QCOMPARE(kReturnPressedSpy[0][0].toString(), QString("Hello world"));
    }

    void testPaste()
    {
        const QString origText = QApplication::clipboard()->text();
        const QString pastedText = "Test paste from klineedit_unittest";
        QApplication::clipboard()->setText(pastedText);
        KLineEdit w;
        w.setText("Hello world");
        w.selectAll();
        QTest::keyClick(&w, Qt::Key_V, Qt::ControlModifier);
        QCOMPARE(w.text(), pastedText);
        QApplication::clipboard()->setText(origText);
    }

};

QTEST_KDEMAIN(KLineEdit_UnitTest, GUI)

#include "klineedit_unittest.moc"
