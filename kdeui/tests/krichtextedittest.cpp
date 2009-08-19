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

#include <qtestevent.h>
#include <qtest_kde.h>

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
