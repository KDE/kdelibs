/* This file is part of the KDE libraries

    Copyright (c) 2008 David Faure <faure@kde.org>

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
#include <ktextedit.h>

class KTextEdit_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testPaste()
    {
        const QString origText = QApplication::clipboard()->text();
        const QString pastedText = "Test paste from ktextedit_unittest";
        QApplication::clipboard()->setText(pastedText);
        KTextEdit w;
        w.setPlainText("Hello world");
        w.selectAll();
        QTest::keyClick(&w, Qt::Key_V, Qt::ControlModifier);
        QCOMPARE(w.toPlainText(), pastedText);
        QApplication::clipboard()->setText(origText);
    }

};

QTEST_KDEMAIN(KTextEdit_UnitTest, GUI)

#include "ktextedit_unittest.moc"


