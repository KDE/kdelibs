/* This file is part of the KDE libraries
    Copyright (c) 2006 David Faure <faure@kde.org>

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

#include "qtest_kde.h"
#include "kstandarddirstest.h"
#include "kstandarddirstest.moc"

QTEST_KDEMAIN_CORE( KStandarddirsTest )

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kglobal.h>
#include <QtCore/QDebug>
#include <config.h>

void KStandarddirsTest::testLocateLocal()
{
    const QString configLocal = KStandardDirs::locateLocal( "config", "ksomethingrc" );
    // KStandardDirs resolves symlinks, so we must compare with canonicalPath()
    QCOMPARE( configLocal, QDir::home().canonicalPath() + "/.kde-unit-test/share/config/ksomethingrc" );
}

void KStandarddirsTest::testSaveLocation()
{
    const QString saveLoc = KGlobal::dirs()->saveLocation( "appdata" );
    QCOMPARE( saveLoc, QDir::home().canonicalPath() + "/.kde-unit-test/share/apps/qttest/" );
}

void KStandarddirsTest::testAppData()
{
    // In addition to testSaveLocation(), we want to also check other KComponentDatas
    KComponentData cData("foo");
    const QString fooAppData = cData.dirs()->saveLocation( "appdata" );
    QCOMPARE( fooAppData, QDir::home().canonicalPath() + "/.kde-unit-test/share/apps/foo/" );
}

static bool isKdelibsInstalled()
{
    // If there's only one dir, it's the local one (~/.kde-unit-test/share/apps/),
    // meaning that kdelibs wasn't installed (or we don't find where, the environment isn't right).
    return KGlobal::dirs()->resourceDirs( "data" ).count() > 1;
}

void KStandarddirsTest::testFindResource()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed", SkipAll );

#ifdef Q_WS_WIN
#define EXT ".exe"
#else
#define EXT ""
#endif
    const QString bin = KGlobal::dirs()->findResource( "exe", "kioslave" EXT );
    QVERIFY( !bin.isEmpty() );
    QVERIFY( bin.endsWith( "kde4/libexec/kioslave" EXT ) );
    QVERIFY( !QDir::isRelativePath(bin) );

    const QString data = KGlobal::dirs()->findResource( "data", "katepart/syntax/sql.xml" );
    QVERIFY( !data.isEmpty() );
    QVERIFY( data.endsWith( "share/apps/katepart/syntax/sql.xml" ) );
    QVERIFY( !QDir::isRelativePath(data) );
}

static bool oneEndsWith( const QStringList& lst, const QString& str)
{
    for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
        if ( (*it).endsWith( str ) )
            return true;
    }
    return false;
}

void KStandarddirsTest::testFindAllResources()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed", SkipAll );

    const QStringList kateSyntaxFiles = KGlobal::dirs()->findAllResources( "data", "katepart/syntax/" );
    QVERIFY( !kateSyntaxFiles.isEmpty() );
    QVERIFY( kateSyntaxFiles.count() > 80 ); // I have 130 here, installed by kdelibs.

    const QStringList configFiles = KGlobal::dirs()->findAllResources( "config" );
    QVERIFY( !configFiles.isEmpty() );
    QVERIFY( configFiles.count() > 5 ); // I have 9 here
    QVERIFY( oneEndsWith( configFiles, "share/config/kdebugrc" ) );
    QVERIFY( !oneEndsWith( configFiles, "share/config/colors/Web.colors" ) ); // recursive was false

    const QStringList configFilesRecursive = KGlobal::dirs()->findAllResources( "config", QString(),
                                                                                KStandardDirs::Recursive );
    QVERIFY( !configFilesRecursive.isEmpty() );
    QVERIFY( configFilesRecursive.count() > 5 ); // I have 15 here
    QVERIFY( oneEndsWith( configFilesRecursive, "share/config/kdebugrc" ) );
    QVERIFY( oneEndsWith( configFilesRecursive, "share/config/colors/Web.colors" ) ); // proves that recursive worked

    const QStringList configFilesRecursiveWithFilter = KGlobal::dirs()->findAllResources( "config", "*rc",
                                                                                          KStandardDirs::Recursive );
    QVERIFY( !configFilesRecursiveWithFilter.isEmpty() );
    QVERIFY( configFilesRecursiveWithFilter.count() >= 5 ); // back to ~ 9
    QVERIFY( oneEndsWith( configFilesRecursiveWithFilter, "share/config/kdebugrc" ) );
    QVERIFY( !oneEndsWith( configFilesRecursiveWithFilter, "share/config/colors/Web.colors" ) ); // didn't match the filter

#if 0
    list = t.findAllResources("html", "en/*/index.html", false);
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
        kDebug() << "docs " << (*it).toAscii().constData();
    }

    list = t.findAllResources("html", "*/*/*.html", false);
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
        kDebug() << "docs " << (*it).toAscii().constData();
    }
#endif
}

void KStandarddirsTest::testFindDirs()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed", SkipAll );

    const QString t = KStandardDirs::locateLocal("data", "kconf_update/" );
    const QStringList dirs = KGlobal::dirs()->findDirs( "data", "kconf_update" );
    QVERIFY( !dirs.isEmpty() );
    QVERIFY( dirs.count() >= 2 ); // at least local and global
    //qDebug() << dirs;
}

void KStandarddirsTest::testFindResourceDir()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed", SkipAll );

    const QString configDir = KGlobal::dirs()->findResourceDir( "config", "kdebugrc" );
    QVERIFY( !configDir.isEmpty() );
    QVERIFY( configDir.endsWith( "/config/" ) );
}

void KStandarddirsTest::testFindExe()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed", SkipAll );

    // findExe with a result in bin
    const QString kded = KGlobal::dirs()->findExe( "kded4" );
    QVERIFY( !kded.isEmpty() );
    QVERIFY( kded.endsWith( "bin/kded4" EXT ) );

    // findExe with a result in libexec
    const QString lnusertemp = KGlobal::dirs()->findExe( "lnusertemp" );
    QVERIFY( !lnusertemp.isEmpty() );
    QVERIFY( lnusertemp.endsWith( "lib" KDELIBSUFF "/kde4/libexec/lnusertemp" EXT ) );

    // Check the "exe" resource too
    QCOMPARE( KGlobal::dirs()->realFilePath(lnusertemp),
              KGlobal::dirs()->locate( "exe", "lnusertemp" ) );

#ifdef Q_OS_UNIX
    const QString pwd = QDir::currentPath();
    QDir::setCurrent("/bin");
    QCOMPARE(KGlobal::dirs()->findExe("./sh"), QString::fromLatin1("/bin/sh"));
    QDir::setCurrent(pwd);
#endif
}

void KStandarddirsTest::testLocate()
{
    const QString textPlain = "/usr/share/mime/text/x-patch.xml";
    if (!QFile::exists(textPlain))
        QSKIP("xdg-share-mime not installed", SkipAll);

    const QString res = KGlobal::dirs()->locate("xdgdata-mime", "text/x-patch.xml");
    QCOMPARE(res, textPlain);
}

void KStandarddirsTest::testRelativeLocation()
{
    const QString file = "kdebugrc";
    QString located = KGlobal::dirs()->locate( "config", file );
    QCOMPARE( KGlobal::dirs()->relativeLocation( "config", located ), file );
}

void KStandarddirsTest::testAddResourceType()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed", SkipAll );

    QString ret = KStandardDirs::locate( "dtd", "customization/catalog" );
    QCOMPARE(ret, QString()); // normal, there's no "dtd" resource in kstandarddirs by default

    KGlobal::dirs()->addResourceType("dtd", "data", "ksgmltools2/");
    ret = KStandardDirs::locate( "dtd", "customization/catalog" );
    QVERIFY(!ret.isEmpty());
}

void KStandarddirsTest::testAddResourceDir()
{
    const QString dir = QString::fromLatin1(KDESRCDIR);
    const QString file = "Cairo";
    QString ret = KStandardDirs::locate( "here", file );
    QCOMPARE(ret, QString()); // not set up yet

    KGlobal::dirs()->addResourceDir("here", dir);
    ret = KStandardDirs::locate( "here", file );
    QCOMPARE(ret, KStandardDirs::realPath(dir) + "Cairo");
}
