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

#include <QtTest/qttest_kde.h>
#include <kprotocolinfo.h>

static void checkIcon( const KURL& url, const QString& expectedIcon )
{
    QString icon = KMimeType::iconNameForURL( url );
    COMPARE( icon, expectedIcon );
}

QTTEST_KDEMAIN( KMimeTypeTest, NoGUI )

void KMimeTypeTest::testByName()
{
    if ( !KSycoca::isAvailable() )
        SKIP( "ksycoca not available", SkipAll );

    KMimeType::Ptr s0 = KMimeType::mimeType("application/x-zerosize");
    VERIFY( s0 );
    COMPARE( s0->name(), QString::fromLatin1("application/x-zerosize") );
    qDebug("Comment is %s", qPrintable(s0->comment(KURL(),false)) );

    KMimeType::Ptr s1 = KMimeType::mimeType("text/plain");
    VERIFY( s1 );
    COMPARE( s1->name(), QString::fromLatin1("text/plain") );
    qDebug("Comment is %s", qPrintable(s1->comment(KURL(),false)) );
}

void KMimeTypeTest::testIcons()
{
    if ( !KSycoca::isAvailable() )
        SKIP( "ksycoca not available", SkipAll );

    // Obviously those tests will need to be fixed if we ever change the name of the icons
    // but at least they unit-test KMimeType::iconNameForURL.
    checkIcon( "file:/tmp/", "folder" );
    checkIcon( "file:/root/", "folder_locked" );
}

void KMimeTypeTest::testFindByPath()
{
    KMimeType::Ptr mf;

    mf = KMimeType::findByPath( KStandardDirs::findExe( "kioexec" ) );
    VERIFY( mf );
    COMPARE( mf->name(), QString::fromLatin1( "application/x-executable" ) );

    // Can't use KIconLoader since this is a "without GUI" test.
    QString fh = locate( "icon", "crystalsvg/22x22/filesystems/folder_home.png" );
    VERIFY( !fh.isEmpty() );
    mf = KMimeType::findByURL( fh, 0, true, false );
    VERIFY( mf );
    COMPARE( mf->name(), QString::fromLatin1( "image/png" ) );
}

void KMimeTypeTest::testFindByURL()
{
    KMimeType::Ptr mf;

    mf = KMimeType::findByURL( KURL("http://foo/bar.png") );
    VERIFY( mf );
    COMPARE( mf->name(), QString::fromLatin1( "application/octet-stream" ) ); // HTTP can't know before downloading

    if ( !KProtocolInfo::isKnownProtocol(KURL("man:/")) )
        SKIP( "man protocol not installed", SkipSingle );

    mf = KMimeType::findByURL( KURL("man:/ls") );
    VERIFY( mf );
    COMPARE( mf->name(), QString::fromLatin1("text/html") );

    mf = KMimeType::findByURL( KURL("man:/ls/") );
    VERIFY( mf );
    COMPARE( mf->name(), QString::fromLatin1("text/html") );
}

void KMimeTypeTest::testAllMimeTypes()
{
    if ( !KSycoca::isAvailable() )
        SKIP( "ksycoca not available", SkipAll );

    const KMimeType::List lst = KMimeType::allMimeTypes();
    VERIFY( !lst.isEmpty() );
    const KServiceType::List allServiceTypes = KServiceType::allServiceTypes();
    VERIFY( allServiceTypes.count() >= lst.count() ); // all mimetypes are in the allServiceTypes list.

    for ( KMimeType::List::ConstIterator it = lst.begin();
          it != lst.end(); ++it ) {
        const KMimeType::Ptr mime = (*it);
        const QString name = mime->name();
        //qDebug( "%s", qPrintable( name ) );
        VERIFY( !name.isEmpty() );
        VERIFY( mime->isType( KST_KMimeType ) );

        const KMimeType::Ptr lookedupMime = KMimeType::mimeType( name );
        VERIFY( lookedupMime ); // not null
        COMPARE( lookedupMime->name(), name );

        const KServiceType::Ptr lookedupServiceType = KMimeType::serviceType( name );
        VERIFY( lookedupServiceType ); // not null
        COMPARE( lookedupServiceType->name(), name );

        // Check that the mimetype is part of the allServiceTypes list (by name)
        KServiceType::List::ConstIterator stit = allServiceTypes.begin();
        const KServiceType::List::ConstIterator stend = allServiceTypes.end();
        bool found = false;
        for ( ; !found && stit != stend; ++stit ) {
            found = ( (*stit)->name() == name );
        }
    }

    // A bit of checking on the allServiceTypes list itself
    KServiceType::List::ConstIterator stit = allServiceTypes.begin();
    const KServiceType::List::ConstIterator stend = allServiceTypes.end();
    for ( ; stit != stend; ++stit ) {
        const KServiceType::Ptr servtype = (*stit);
        const QString name = servtype->name();
        VERIFY( !name.isEmpty() );
        // It's a pure servicetype, or a mimetype, or mimetype-derivative.
        VERIFY( servtype->sycocaType() == KST_KServiceType || servtype->isType( KST_KMimeType ) );
        if ( servtype->sycocaType() == KST_KServiceType ) {
            //qDebug( "%s", qPrintable( name ) );
        }
    }
}

void KMimeTypeTest::testAllServices()
{
    if ( !KSycoca::isAvailable() )
        SKIP( "ksycoca not available", SkipAll );
    const KService::List lst = KService::allServices();
    VERIFY( !lst.isEmpty() );

    for ( KService::List::ConstIterator it = lst.begin();
          it != lst.end(); ++it ) {
        const KService::Ptr service = (*it);
        VERIFY( service->isType( KST_KService ) );

        const QString type = service->type();
        VERIFY( !type.isEmpty() );
        VERIFY( type == "Application" || type == "Service" );
        const QString name = service->name();
        const QString dep = service->desktopEntryPath();
        qDebug( "%s %s", qPrintable( name ), qPrintable( dep ) );
        VERIFY( !name.isEmpty() );
        VERIFY( !dep.isEmpty() );

        KService::Ptr lookedupService = KService::serviceByDesktopPath( dep );
        VERIFY( lookedupService ); // not null
        COMPARE( lookedupService->desktopEntryPath(), dep );

        if ( type == "Application" )
        {
            const QString menuId = service->menuId();
            VERIFY( !menuId.isEmpty() );
            lookedupService = KService::serviceByMenuId( menuId );
            VERIFY( lookedupService ); // not null
            COMPARE( lookedupService->menuId(), menuId );
        }
    }
}

void KMimeTypeTest::testAllInitServices()
{
    if ( !KSycoca::isAvailable() )
        SKIP( "ksycoca not available", SkipAll );
    const KService::List lst = KService::allInitServices();
    if ( lst.isEmpty() )
        SKIP( "no init services available", SkipAll ); // this happens when only kdelibs is installed

    for ( KService::List::ConstIterator it = lst.begin();
          it != lst.end(); ++it ) {
        const KService::Ptr service = (*it);
        VERIFY( service->isType( KST_KService ) );

        const QString name = service->name();
        const QString dep = service->desktopEntryPath();
        qDebug( "%s %s (type=%s init=%s)", qPrintable( name ), qPrintable( dep ), qPrintable( service->type() ), qPrintable( service->init() ) );
        VERIFY( !name.isEmpty() );
        VERIFY( !dep.isEmpty() );

        const QString init = service->init();
        VERIFY( !init.isEmpty() ); // kbuildservicefactory.cpp ensures that only services with init not empty are put in the init list
    }
}

