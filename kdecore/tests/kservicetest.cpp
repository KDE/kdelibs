/*
 *  Copyright (C) 2006 David Faure   <faure@kde.org>
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

#include "kservicetest.h"
#include "kservicetest.moc"
#include <ksycoca.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qtest_kde.h>
#include <kprotocolinfo.h>
#include <kservicetypetrader.h>
#include <kprocess.h>

void KServiceTest::initTestCase()
{
    QString profilerc = KStandardDirs::locateLocal( "config", "profilerc" );
    if ( !profilerc.isEmpty() )
        QFile::remove( profilerc );

    if ( !KSycoca::isAvailable() ) {
        // Create ksycoca in ~/.kde-unit-test
        KProcess proc;
        proc.setEnvironment( "KDEHOME", QFile::decodeName( getenv( "KDEHOME" ) ) );
        proc << "kbuildsycoca" << "--noincremental";
        proc.start( KProcess::Block );
    }
}

QTEST_KDEMAIN( KServiceTest, NoGUI )

void KServiceTest::testByName()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    KServiceType::Ptr s0 = KServiceType::serviceType("KParts/ReadOnlyPart");
    QVERIFY( s0 );
    QCOMPARE( s0->name(), QString::fromLatin1("KParts/ReadOnlyPart") );
}

void KServiceTest::testAllServiceTypes()
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

void KServiceTest::testAllServices()
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
        //qDebug( "%s %s", qPrintable( name ), qPrintable( dep ) );
        QVERIFY( !name.isEmpty() );
        QVERIFY( !dep.isEmpty() );

        KService::Ptr lookedupService = KService::serviceByDesktopPath( dep );
        QVERIFY( lookedupService ); // not null
        QCOMPARE( lookedupService->desktopEntryPath(), dep );

        if ( type == "Application" )
        {
            const QString menuId = service->menuId();
            if ( menuId.isEmpty() )
                qWarning( "%s has an empty menuId!", qPrintable( dep ) );
            QVERIFY( !menuId.isEmpty() );
            lookedupService = KService::serviceByMenuId( menuId );
            QVERIFY( lookedupService ); // not null
            QCOMPARE( lookedupService->menuId(), menuId );
        }
    }
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

void KServiceTest::testServiceTypeTraderForReadOnlyPart()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll );

    // Querying trader for services associated with KParts/ReadOnlyPart
    KService::List offers = KServiceTypeTrader::self()->query("KParts/ReadOnlyPart");
    QVERIFY( offers.count() > 0 );
    //foreach( KService::Ptr service, offers )
    //    qDebug( "%s %s", qPrintable( service->name() ), qPrintable( service->desktopEntryPath() ) );

    // Only test for parts provided by kdelibs:
    QVERIFY( offerListHasService( offers, "katepart.desktop" ) );
    QVERIFY( offerListHasService( offers, "kmultipart.desktop" ) );
    QVERIFY( offerListHasService( offers, "khtml.desktop" ) );
    QVERIFY( offerListHasService( offers, "khtmlimage.desktop" ) );
    QVERIFY( offerListHasService( offers, "kjavaappletviewer.desktop" ) );
    QVERIFY( offerListHasService( offers, "kcertpart.desktop" ) );

    // Check ordering according to InitialPreference
    int lastPreference = -1;
    bool lastAllowedAsDefault = true;
    KService::List::const_iterator it = offers.begin();
    for ( ; it != offers.end() ; it++ ) {
        const QString path = (*it)->desktopEntryPath();
        const int preference = (*it)->initialPreference(); // ## might be wrong if we use per-servicetype preferences...
        qDebug( "%s has preference %d, allowAsDefault=%d", qPrintable( path ), preference, (*it)->allowAsDefault() );
        if ( lastAllowedAsDefault && !(*it)->allowAsDefault() ) {
            // first "not allowed as default" offer
            lastAllowedAsDefault = false;
            lastPreference = -1; // restart
        }
        if ( lastPreference != -1 )
            QVERIFY( preference <= lastPreference );
        lastPreference = preference;
    }

    // Now look for any KTextEditor/Plugin
    offers = KServiceTypeTrader::self()->query("KTextEditor/Plugin");
    QVERIFY( offerListHasService( offers, "ktexteditor_isearch.desktop" ) );
    QVERIFY( offerListHasService( offers, "ktexteditor_insertfile.desktop" ) );
}

void KServiceTest::testHasServiceType1() // with services constructed with a full path (rare)
{
    QString katepartPath = KStandardDirs::locate( "services", "katepart.desktop" );
    QVERIFY( !katepartPath.isEmpty() );
    KService katepart( katepartPath );
    QVERIFY( katepart.hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( katepart.hasServiceType( "KParts/ReadWritePart" ) );

    QString ktexteditor_isearchPath = KStandardDirs::locate( "services", "ktexteditor_isearch.desktop" );
    QVERIFY( !ktexteditor_isearchPath.isEmpty() );
    KService ktexteditor_isearch( ktexteditor_isearchPath );
    QVERIFY( ktexteditor_isearch.hasServiceType( "KTextEditor/Plugin" ) );
    QVERIFY( !ktexteditor_isearch.hasServiceType( "KParts/ReadOnlyPart" ) );
}

void KServiceTest::testHasServiceType2() // with services coming from ksycoca
{
    KService::Ptr katepart = KService::serviceByDesktopPath( "katepart.desktop" );
    QVERIFY( !katepart.isNull() );
    QVERIFY( katepart->hasServiceType( "KParts/ReadOnlyPart" ) );
    QVERIFY( katepart->hasServiceType( "KParts/ReadWritePart" ) );

    KService::Ptr ktexteditor_isearch = KService::serviceByDesktopPath( "ktexteditor_isearch.desktop" );
    QVERIFY( !ktexteditor_isearch.isNull() );
    QVERIFY( ktexteditor_isearch->hasServiceType( "KTextEditor/Plugin" ) );
    QVERIFY( !ktexteditor_isearch->hasServiceType( "KParts/ReadOnlyPart" ) );
}

// TODO tests that involve writing a profilerc and checking that the trader is obeying it
