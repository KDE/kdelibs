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
    const KMimeType::List allMimeTypes = KMimeType::allMimeTypes();

    for ( KMimeType::List::ConstIterator it = lst.begin();
          it != lst.end(); ++it ) {
        const KMimeType::Ptr mime = (*it);
        const QString name = mime->name();
        //qDebug( "%s", qPrintable( name ) );
        QVERIFY( !name.isEmpty() );
        QVERIFY( mime->isType( KST_KMimeType ) );

        const KMimeType::Ptr lookedupMime = KMimeType::mimeType( name );
        QVERIFY( lookedupMime ); // not null
        QCOMPARE( lookedupMime->name(), name );

        // Check that the mimetype is part of the allMimeTypes list (by name)
        KMimeType::List::ConstIterator stit = allMimeTypes.begin();
        const KMimeType::List::ConstIterator stend = allMimeTypes.end();
        bool found = false;
        for ( ; !found && stit != stend; ++stit ) {
            found = ( (*stit)->name() == name );
        }
    }

    // A bit of checking on the allMimeTypes list itself
    KMimeType::List::ConstIterator stit = allMimeTypes.begin();
    const KMimeType::List::ConstIterator stend = allMimeTypes.end();
    for ( ; stit != stend; ++stit ) {
        const KMimeType::Ptr mime = (*stit);
        const QString name = mime->name();
        QVERIFY( !name.isEmpty() );
        QCOMPARE( name.count( '/' ), 1 );
        QVERIFY( mime->isType( KST_KMimeType ) );
    }
}

void KMimeTypeTest::testAllServiceTypes()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    const KServiceType::List allServiceTypes = KServiceType::allServiceTypes();

    // A bit of checking on the allServiceTypes list itself
    KServiceType::List::ConstIterator stit = allServiceTypes.begin();
    const KServiceType::List::ConstIterator stend = allServiceTypes.end();
    for ( ; stit != stend; ++stit ) {
        const KServiceType::Ptr servtype = (*stit);
        const QString name = servtype->name();
        QVERIFY( !name.isEmpty() );
        QVERIFY( servtype->sycocaType() == KST_KServiceType );
    }
}

void KMimeTypeTest::testAllServices()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );
    const KService::List lst = KService::allServices();
    QVERIFY( !lst.isEmpty() );

    for ( KService::List::ConstIterator it = lst.begin();
          it != lst.end(); ++it ) {
        const KService::Ptr service = (*it);
        QVERIFY( service->isType( KST_KService ) );

        const QString type = service->type();
        QVERIFY( !type.isEmpty() );
        QVERIFY( type == "Application" || type == "Service" );
        const QString name = service->name();
        const QString dep = service->desktopEntryPath();
        qDebug( "%s %s", qPrintable( name ), qPrintable( dep ) );
        QVERIFY( !name.isEmpty() );
        QVERIFY( !dep.isEmpty() );

        KService::Ptr lookedupService = KService::serviceByDesktopPath( dep );
        QVERIFY( lookedupService ); // not null
        QCOMPARE( lookedupService->desktopEntryPath(), dep );

        if ( type == "Application" )
        {
            const QString menuId = service->menuId();
            QVERIFY( !menuId.isEmpty() );
            lookedupService = KService::serviceByMenuId( menuId );
            QVERIFY( lookedupService ); // not null
            QCOMPARE( lookedupService->menuId(), menuId );
        }
    }
}

void KMimeTypeTest::testAllInitServices()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );
    const KService::List lst = KService::allInitServices();
    if ( lst.isEmpty() )
        QSKIP( "no init services available", SkipAll ); // this happens when only kdelibs is installed

    for ( KService::List::ConstIterator it = lst.begin();
          it != lst.end(); ++it ) {
        const KService::Ptr service = (*it);
        QVERIFY( service->isType( KST_KService ) );

        const QString name = service->name();
        const QString dep = service->desktopEntryPath();
        qDebug( "%s %s (type=%s init=%s)", qPrintable( name ), qPrintable( dep ), qPrintable( service->type() ), qPrintable( service->init() ) );
        QVERIFY( !name.isEmpty() );
        QVERIFY( !dep.isEmpty() );

        const QString init = service->init();
        QVERIFY( !init.isEmpty() ); // kbuildservicefactory.cpp ensures that only services with init not empty are put in the init list
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

void KMimeTypeTest::testServiceTypeTraderForReadOnlyPart()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    // Querying trader for services associated with KParts/ReadOnlyPart
    KService::List offers = KServiceTypeTrader::self()->query("KParts/ReadOnlyPart");
    QVERIFY( offers.count() > 0 );

    // Only test for parts provided by kdelibs:
    QVERIFY( offerListHasService( offers, "katepart.desktop" ) );
    QVERIFY( offerListHasService( offers, "kmultipart.desktop" ) );
    QVERIFY( offerListHasService( offers, "khtml.desktop" ) );
    QVERIFY( offerListHasService( offers, "khtmlimage.desktop" ) );
    QVERIFY( offerListHasService( offers, "kjavaappletviewer.desktop" ) );
    QVERIFY( offerListHasService( offers, "kcertpart.desktop" ) );

    // Now look for any KTextEditor/Plugin
    offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin");
    QVERIFY( offerListHasService( offers, "ktexteditor_isearch.desktop" ) );
    QVERIFY( offerListHasService( offers, "ktexteditor_insertfile.desktop" ) );
}

// TODO tests that involve writing a profilerc and checking that the trader is obeying it
