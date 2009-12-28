/* This file is part of the KDE libraries
    Copyright (c) 2005 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qtest_kde.h>
#include "kbookmarktest.h"
#include "kbookmarktest.moc"

QTEST_KDEMAIN( KBookmarkTest, false )

#include <kbookmark.h>
#include <kdebug.h>
#include <QtCore/QMimeData>

static void compareBookmarks( const KBookmark& initialBookmark, const KBookmark& decodedBookmark )
{
    QCOMPARE( decodedBookmark.url(), initialBookmark.url() );
    QCOMPARE( decodedBookmark.icon(), initialBookmark.icon() );
    QCOMPARE( decodedBookmark.text(), initialBookmark.text() );
    QCOMPARE( decodedBookmark.description(), initialBookmark.description() );
    QDomNamedNodeMap decodedAttribs = decodedBookmark.internalElement().attributes();
    QDomNamedNodeMap initialAttribs = initialBookmark.internalElement().attributes();
    QCOMPARE( decodedAttribs.count(), initialAttribs.count() );
    for ( uint i = 0; i < decodedAttribs.length(); ++i ) {
        QDomAttr decodedAttr = decodedAttribs.item( i ).toAttr();
        QDomAttr initialAttr = initialAttribs.item( i ).toAttr();
        QCOMPARE( decodedAttr.name(), initialAttr.name() );
        QCOMPARE( decodedAttr.value(), initialAttr.value() );
    }
}

void KBookmarkTest::testMimeDataOneBookmark()
{
    QMimeData* mimeData = new QMimeData;

    KBookmark bookmark = KBookmark::standaloneBookmark( "KDE", KUrl( "http://www.kde.org" ), "icon" );
    bookmark.setDescription( "Comment" );
    QVERIFY( !bookmark.isNull() );
    bookmark.populateMimeData( mimeData );

    QDomDocument doc;
    QVERIFY( KUrl::List::canDecode( mimeData ) );
    QVERIFY( KBookmark::List::canDecode( mimeData ) );
    const KBookmark::List decodedBookmarks = KBookmark::List::fromMimeData( mimeData, doc );
    QVERIFY( !decodedBookmarks.isEmpty() );
    QCOMPARE( decodedBookmarks.count(), 1 );
    QVERIFY( !decodedBookmarks[0].isNull() );
    compareBookmarks( bookmark, decodedBookmarks[0] );

    // Do like keditbookmarks's paste code: (CreateCommand::execute)
    // Crashed before passing "doc" to fromMimeData (#160679)
    QDomElement clonedElem = decodedBookmarks[0].internalElement().cloneNode().toElement();

    delete mimeData;
}

void KBookmarkTest::testMimeDataBookmarkList()
{
    QMimeData* mimeData = new QMimeData;

    KBookmark bookmark1 = KBookmark::standaloneBookmark( "KDE", KUrl( "http://www.kde.org" ), "icon" );
    bookmark1.setDescription( "KDE comment" );
    QVERIFY( !bookmark1.isNull() );
    KBookmark bookmark2 = KBookmark::standaloneBookmark( "KOffice", KUrl( "http://www.koffice.org" ), "koicon" );
    bookmark2.setDescription( "KOffice comment" );
    QVERIFY( !bookmark2.isNull() );
    bookmark2.setMetaDataItem( "key", "value" );

    KBookmark::List initialBookmarks;
    initialBookmarks.append( bookmark1 );
    initialBookmarks.append( bookmark2 );
    initialBookmarks.populateMimeData( mimeData );

    QDomDocument doc;
    QVERIFY( KUrl::List::canDecode( mimeData ) );
    QVERIFY( KBookmark::List::canDecode( mimeData ) );
    const KBookmark::List decodedBookmarks = KBookmark::List::fromMimeData( mimeData, doc );
    QCOMPARE( decodedBookmarks.count(), 2 );
    QVERIFY( !decodedBookmarks[0].isNull() );
    QVERIFY( !decodedBookmarks[1].isNull() );
    compareBookmarks( bookmark1, decodedBookmarks[0] );
    compareBookmarks( bookmark2, decodedBookmarks[1] );

    delete mimeData;
}
