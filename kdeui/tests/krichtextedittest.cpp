/* This file is part of the KDE libraries
    Copyright (c) 2009 Thomas McGuire <mcguire@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "krichtextedittest.h"

#include <krichtextedit.h>
#include <kcolorscheme.h>

#include <qtestevent.h>
#include <qtest_kde.h>
#include <qtextcursor.h>
#include <qfont.h>

QTEST_KDEMAIN(KRichTextEditTest, GUI)

void KRichTextEditTest::testLinebreaks()
{
  KRichTextEdit edit;
  edit.enableRichTextMode();

  // Enter the text with keypresses, for some strange reason a normal setText() or
  // setPlainText() call doesn't do the trick
  QTest::keyClick( &edit, Qt::Key_A );
  QTest::keyClick( &edit, Qt::Key_Enter );
  QTest::keyClick( &edit, Qt::Key_Enter );
  edit.setTextUnderline( true );
  QTest::keyClick( &edit, Qt::Key_B );
  QTest::keyClick( &edit, Qt::Key_Enter );
  QTest::keyClick( &edit, Qt::Key_Enter );
  QTest::keyClick( &edit, Qt::Key_Enter );
  QTest::keyClick( &edit, Qt::Key_C );
  QCOMPARE( edit.toPlainText(), QString( "a\n\nb\n\n\nc" ) );

  QString html = edit.toCleanHtml();
  edit.clear();
  edit.setHtml( html );
  QCOMPARE( edit.toPlainText(), QString( "a\n\nb\n\n\nc" ) );
}

void KRichTextEditTest::testUpdateLinkAdd()
{
    KRichTextEdit edit;
    edit.enableRichTextMode();

    // Add text, apply initial formatting, and add a link
    QTextCursor cursor = edit.textCursor();
    cursor.insertText(QString("Test"));
    cursor.select(QTextCursor::BlockUnderCursor);
    edit.setTextCursor(cursor);
    edit.setTextBold(true);
    edit.setTextItalic(true);
    edit.updateLink(QString("http://www.kde.org"), QString("KDE"));

    // Validate text and formatting
    cursor.movePosition(QTextCursor::Start);
    cursor.select(QTextCursor::WordUnderCursor);
    edit.setTextCursor(cursor);
    QCOMPARE(edit.toPlainText(), QString("KDE "));
    QCOMPARE(edit.fontItalic(), true);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    QCOMPARE(edit.fontUnderline(), true);
    QTextCharFormat charFormat = cursor.charFormat();
    QCOMPARE(charFormat.underlineColor(), KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color());
    QCOMPARE(charFormat.underlineStyle(), QTextCharFormat::SingleUnderline);
    QCOMPARE(charFormat.foreground(), QBrush(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color()));
}

void KRichTextEditTest::testUpdateLinkRemove()
{
    KRichTextEdit edit;
    edit.enableRichTextMode();

    // Add text, apply initial formatting, and add a link
    QTextCursor cursor = edit.textCursor();
    cursor.insertText(QString("Test"));
    cursor.select(QTextCursor::BlockUnderCursor);
    edit.setTextCursor(cursor);
    edit.setTextBold(true);
    edit.setTextItalic(true);
    edit.updateLink(QString("http://www.kde.org"), QString("KDE"));

    // Remove link and validate formatting
    cursor.movePosition(QTextCursor::Start);
    cursor.select(QTextCursor::WordUnderCursor);
    edit.setTextCursor(cursor);
    edit.updateLink(QString(), QString("KDE"));
    cursor.movePosition(QTextCursor::Start);
    cursor.select(QTextCursor::WordUnderCursor);
    edit.setTextCursor(cursor);
    QCOMPARE(edit.toPlainText(), QString("KDE "));
    QCOMPARE(edit.fontItalic(), true);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    QCOMPARE(edit.fontUnderline(), false);
    QTextCharFormat charFormat = cursor.charFormat();
    QCOMPARE(charFormat.underlineColor(), KColorScheme(QPalette::Active, KColorScheme::View).foreground().color());
    QCOMPARE(charFormat.underlineStyle(), QTextCharFormat::NoUnderline);
    QCOMPARE(charFormat.foreground(), QBrush(KColorScheme(QPalette::Active, KColorScheme::View).foreground().color()));
}
