/*
 *  Copyright 2003, 2008 David Faure   <faure@kde.org>
 *
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <QFontMetrics>
#include <qtest_kde.h>

#include <kdebug.h>
#include <QObject>
#include "kwordwrap.h"

class KWordWrap_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void oldTruncationTest()
    {
        QFont font( "helvetica", 12 ); // let's hope we all have the same...
        QFontMetrics fm( font );
        QRect r( 0, 0, 100, -1 );
        QString str = "test wadabada [/foo/bar/waba]";
        KWordWrap* ww = KWordWrap::formatText( fm, r, 0, str );
        kDebug() << str << " => " << ww->truncatedString();
        QVERIFY(ww->truncatedString().endsWith("..."));
        delete ww;

        str = "</p></p></p></p>";
        for ( ; r.width() > 0 ; r.setWidth( r.width()-10 ) )
        {
            ww = KWordWrap::formatText( fm, r, 0, str );
            kDebug() << str << " => " << ww->truncatedString();
            QVERIFY(ww->truncatedString().endsWith("..."));
            delete ww;
        }
    }

    void testWithExistingNewlines() // when the input string has \n already
    {
        QRect r( 0, 0, 1000, -1 ); // very wide
        QFont font( "helvetica", 12 ); // let's hope we all have the same...
        QFontMetrics fm( font );
        QString inputString = "The title here\nFoo (bar)\nFoo2 (bar2)";
        KWordWrap* ww = KWordWrap::formatText( fm, r, 0, inputString );
        QString str = ww->wrappedString();
        QCOMPARE(str, inputString);
        delete ww;
    }
};

QTEST_KDEMAIN(KWordWrap_UnitTest, GUI)

#include "kwordwraptest.moc"
