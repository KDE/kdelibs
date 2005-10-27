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
