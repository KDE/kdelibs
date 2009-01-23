/* This file is part of the KDE libraries

    Copyright (c) 2008 David Faure <faure@kde.org>
    Copyright 2008 Stephen Kelly <steveire@gmail.com>

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
    void testPaste();
    // These tests are probably invalid due to using invalid html.
//     void testImportWithHorizontalTraversal();
//     void testImportWithVerticalTraversal();
//     void testBrInsideParagraphThroughTextEdit();

};

void KTextEdit_UnitTest::testPaste()
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

// void KTextEdit_UnitTest::testImportWithVerticalTraversal()
// {
//     QTextEdit *te = new QTextEdit();
//
//     te->setHtml("<p>Foo</p><br /><br /><br /><p>Bar</p>");
//
//     QTextCursor cursor = te->textCursor();
//     cursor.movePosition(QTextCursor::Start);
//     QVERIFY(cursor.block().text() == QString( "Foo" ));
//     cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 4);
//
//     // Cursor is at the beginning of the block.
//     QVERIFY(cursor.block().position() == cursor.position());
//     QVERIFY(cursor.block().text() == QString( "Bar" ));
// }
//
// void KTextEdit_UnitTest::testImportWithHorizontalTraversal()
// {
//     QTextEdit *te = new QTextEdit();
//
//     te->setHtml("<p>Foo</p><br /><p>Bar</p>");
//
//     // br elements should be represented just like empty paragraphs.
//
//     QTextCursor cursor = te->textCursor();
//     cursor.movePosition(QTextCursor::Start);
//     QVERIFY(cursor.block().text() == QString( "Foo" ));
//     cursor.movePosition(QTextCursor::EndOfBlock);
//     cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 2);
//
//     // Cursor is at the beginning of the block.
//     QVERIFY(cursor.block().position() == cursor.position());
//     QVERIFY(cursor.block().text() == QString( "Bar" ));
// }
//
// void KTextEdit_UnitTest::testBrInsideParagraphThroughTextEdit()
// {
//     QSKIP("This is worked around during export", SkipSingle);
//     QTextEdit *te = new QTextEdit();
//
//     te->setHtml("<p>Foo<br />Bar</p>");
//
//     // br elements inside paragraphs should be a single linebreak.
//
//     QTextCursor cursor = te->textCursor();
//     cursor.movePosition(QTextCursor::Start);
//
//     // This doesn't work, because Qt puts Foo and Bar in the same block, separated by a QChar::LineSeparator
//
//     QVERIFY(cursor.block().text() == QString( "Foo" ));
//     cursor.movePosition(QTextCursor::EndOfBlock);
//     cursor.movePosition(QTextCursor::Right);
//
//     // Cursor is at the beginning of the block.
//     QVERIFY(cursor.block().position() == cursor.position());
//     QVERIFY(cursor.block().text() == QString( "Bar" ));
//
// }


QTEST_KDEMAIN(KTextEdit_UnitTest, GUI)

#include "ktextedit_unittest.moc"


