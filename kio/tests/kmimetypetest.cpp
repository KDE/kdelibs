/*
 *  Copyright (C) 2005 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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

#include "kmimetypetest.h"
#include "kmimetypetest.moc"
#include <kmimetype.h>
#include <ksycoca.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qtest_kde.h>
#include <kprotocolinfo.h>
#include <kmimetypetrader.h>
#include <kservicetypetrader.h>
#include <kprocess.h>

void KMimeTypeTest::initTestCase()
{
    if ( !KSycoca::isAvailable() ) {
        // Create ksycoca in ~/.kde-unit-test
        KProcess proc;
        proc.setEnvironment( "KDEHOME", QFile::decodeName( getenv( "KDEHOME" ) ) );
        proc << "kbuildsycoca" << "--noincremental";
        proc.start( KProcess::Block );
    }
}

static void checkIcon( const KUrl& url, const QString& expectedIcon )
{
    QString icon = KMimeType::iconNameForURL( url );
    QCOMPARE( icon, expectedIcon );
}

QTEST_KDEMAIN( KMimeTypeTest, NoGUI )

void KMimeTypeTest::testByName()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    KMimeType::Ptr s0 = KMimeType::mimeType("application/x-zerosize");
    QVERIFY( s0 );
    QCOMPARE( s0->name(), QString::fromLatin1("application/x-zerosize") );
    qDebug("Comment is %s", qPrintable(s0->comment()) );

    KMimeType::Ptr s1 = KMimeType::mimeType("text/plain");
    QVERIFY( s1 );
    QCOMPARE( s1->name(), QString::fromLatin1("text/plain") );
    qDebug("Comment is %s", qPrintable(s1->comment()) );
}

void KMimeTypeTest::testIcons()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    // Obviously those tests will need to be fixed if we ever change the name of the icons
    // but at least they unit-test KMimeType::iconNameForURL.
    checkIcon( KUrl( "file:/tmp/" ), "folder" );
    if ( QFile::exists( "/root/.ssh/" ) )
        checkIcon( KUrl( "file:/root/.ssh/" ), "folder_locked" );
}

void KMimeTypeTest::testFindByPath()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    KMimeType::Ptr mf;

    QString exePath = KStandardDirs::findExe( "kioexec" );
    if ( exePath.isEmpty() )
        QSKIP( "kioexec not found", SkipAll );

    mf = KMimeType::findByPath( exePath );
    QVERIFY( mf );
    QCOMPARE( mf->name(), QString::fromLatin1( "application/x-executable" ) );

    // Can't use KIconLoader since this is a "without GUI" test.
    QString fh = locate( "icon", "crystalsvg/22x22/filesystems/folder_home.png" );
    QVERIFY( !fh.isEmpty() );
    mf = KMimeType::findByPath( fh );
    QVERIFY( mf );
    QCOMPARE( mf->name(), QString::fromLatin1( "image/png" ) );
}

void KMimeTypeTest::testFindByURL()
{
    KMimeType::Ptr mf;

    mf = KMimeType::findByURL( KUrl("http://foo/bar.png") );
    QVERIFY( mf );
    QCOMPARE( mf->name(), QString::fromLatin1( "application/octet-stream" ) ); // HTTP can't know before downloading

    if ( !KProtocolInfo::isKnownProtocol(KUrl("man:/")) )
        QSKIP( "man protocol not installed", SkipSingle );

    mf = KMimeType::findByURL( KUrl("man:/ls") );
    QVERIFY( mf );
    QCOMPARE( mf->name(), QString::fromLatin1("text/html") );

    mf = KMimeType::findByURL( KUrl("man:/ls/") );
    QVERIFY( mf );
    QCOMPARE( mf->name(), QString::fromLatin1("text/html") );
}

void KMimeTypeTest::testAllMimeTypes()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    const KMimeType::List lst = KMimeType::allMimeTypes();
    QVERIFY( !lst.isEmpty() );

    for ( KMimeType::List::ConstIterator it = lst.begin();
          it != lst.end(); ++it ) {
        const KMimeType::Ptr mime = (*it);
        const QString name = mime->name();
        qDebug( "%s", qPrintable( name ) );
        QVERIFY( !name.isEmpty() );
        QCOMPARE( name.count( '/' ), 1 );
        QVERIFY( mime->isType( KST_KMimeType ) );

        const KMimeType::Ptr lookedupMime = KMimeType::mimeType( name );
        QVERIFY( lookedupMime ); // not null
        QCOMPARE( lookedupMime->name(), name );
    }
}

void KMimeTypeTest::testMimeTypeParent()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    // Check that text/x-diff knows that inherits from text/plain
    const KMimeType::Ptr plain = KMimeType::mimeType( "text/plain" );
    const KMimeType::Ptr derived = KMimeType::mimeType( "text/x-diff" );
    QVERIFY( derived );
    QCOMPARE( derived->parentMimeType(), plain->name() );
}

// Helper method for all the trader tests
static bool offerListHasService( const KService::List& offers,
                                 const QString& desktopEntryPath )
{
    bool found = false;
    KService::List::const_iterator it = offers.begin();
    for ( ; it != offers.end() ; it++ )
    {
        if ( (*it)->desktopEntryPath() == desktopEntryPath ) {
            if( found ) { // should be there only once
                qWarning( "ERROR: %s was found twice in the list", qPrintable( desktopEntryPath ) );
                return false; // make test fail
            }
            found = true;
        }
    }
    return found;
}

void KMimeTypeTest::testMimeTypeTraderForTextPlain()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    // Querying mimetype trader for services associated with text/plain
    KService::List offers = KMimeTypeTrader::self()->query("text/plain", "KParts/ReadOnlyPart");
    QVERIFY( offerListHasService( offers, "katepart.desktop" ) );

    offers = KMimeTypeTrader::self()->query("text/plain", "KTextEditor/Plugin");
    QVERIFY( offers.count() > 0 );

    // We should have at least a few kate plugins like
    // ktexteditor_isearch or ktexteditor_insertfile. This is all from kdelibs.
    QVERIFY( offerListHasService( offers, "ktexteditor_isearch.desktop" ) );
    QVERIFY( offerListHasService( offers, "ktexteditor_insertfile.desktop" ) );
}

void KMimeTypeTest::testMimeTypeTraderForDerivedMimeType()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    // Querying mimetype trader for services associated with text/x-diff, which inherits from text/plain
    KService::List offers = KMimeTypeTrader::self()->query("text/x-diff", "KParts/ReadOnlyPart");
    QVERIFY( offerListHasService( offers, "katepart.desktop" ) );

    offers = KMimeTypeTrader::self()->query("text/x-diff", "KTextEditor/Plugin");
    QVERIFY( offers.count() > 0 );

    // We should have at least a few kate plugins like
    // ktexteditor_isearch or ktexteditor_insertfile. This is all from kdelibs.
    QVERIFY( offerListHasService( offers, "ktexteditor_isearch.desktop" ) );
    QVERIFY( offerListHasService( offers, "ktexteditor_insertfile.desktop" ) );
}

void KMimeTypeTest::testHasServiceType1() // with services constructed with a full path (rare)
{
    QString katepartPath = locate( "services", "katepart.desktop" );
    QVERIFY( !katepartPath.isEmpty() );
    KService katepart( katepartPath );
    QVERIFY( katepart.hasMimeType( KMimeType::mimeType( "text/plain" ).data() ) );
    //QVERIFY( katepart.hasMimeType( KMimeType::mimeType( "text/x-diff" ).data() ) ); // inherited mimetype; fails
    QVERIFY( !katepart.hasMimeType( KMimeType::mimeType( "image/png" ).data() ) );
    QVERIFY( katepart.hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( katepart.hasServiceType( "KParts/ReadWritePart" ) );

    QString ktexteditor_isearchPath = locate( "services", "ktexteditor_isearch.desktop" );
    QVERIFY( !ktexteditor_isearchPath.isEmpty() );
    KService ktexteditor_isearch( ktexteditor_isearchPath );
    QVERIFY( ktexteditor_isearch.hasMimeType( KMimeType::mimeType( "text/plain" ).data() ) );
    //QVERIFY( ktexteditor_isearch.hasMimeType( KMimeType::mimeType( "text/x-diff" ).data() ) ); // inherited mimetype; fails
    QVERIFY( ktexteditor_isearch.hasServiceType( "KTextEditor/Plugin" ) );
    QVERIFY( !ktexteditor_isearch.hasServiceType( "KParts/ReadOnlyPart" ) );
}

void KMimeTypeTest::testHasServiceType2() // with services coming from ksycoca
{
    KService::Ptr katepart = KService::serviceByDesktopPath( "katepart.desktop" );
    QVERIFY( !katepart.isNull() );
    QVERIFY( katepart->hasMimeType( KMimeType::mimeType( "text/plain" ).data() ) );
    QVERIFY( katepart->hasMimeType( KMimeType::mimeType( "text/x-diff" ).data() ) ); // due to inheritance
    QVERIFY( !katepart->hasMimeType( KMimeType::mimeType( "image/png" ).data() ) );
    QVERIFY( katepart->hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( katepart->hasServiceType( "KParts/ReadWritePart" ) );

    KService::Ptr ktexteditor_isearch = KService::serviceByDesktopPath( "ktexteditor_isearch.desktop" );
    QVERIFY( !ktexteditor_isearch.isNull() );
    QVERIFY( ktexteditor_isearch->hasMimeType( KMimeType::mimeType( "text/plain" ).data() ) );
    QVERIFY( ktexteditor_isearch->hasMimeType( KMimeType::mimeType( "text/x-diff" ).data() ) ); // due to inheritance
    QVERIFY( ktexteditor_isearch->hasServiceType( "KTextEditor/Plugin" ) );
    QVERIFY( !ktexteditor_isearch->hasServiceType( "KParts/ReadOnlyPart" ) );
}

// TODO tests that involve writing a profilerc and checking that the trader is obeying it
