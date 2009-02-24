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
#include "config-prefix.h"
#include <QtCore/QDebug>
#include <kconfiggroup.h>
#include <config.h>

void KStandarddirsTest::initTestCase()
{
    m_kdehome = QDir::home().canonicalPath() + "/.kde-unit-test";
}

void KStandarddirsTest::testLocateLocal()
{
    const QString configLocal = KStandardDirs::locateLocal( "config", "ksomethingrc" );
    // KStandardDirs resolves symlinks, so we must compare with canonicalPath()
    QCOMPARE( configLocal, m_kdehome + "/share/config/ksomethingrc" );
}

void KStandarddirsTest::testSaveLocation()
{
    const QString saveLoc = KGlobal::dirs()->saveLocation( "appdata" );
    QCOMPARE( saveLoc, m_kdehome + "/share/apps/qttest/" );
}

void KStandarddirsTest::testAppData()
{
    // In addition to testSaveLocation(), we want to also check other KComponentDatas
    KComponentData cData("foo");
    const QString fooAppData = cData.dirs()->saveLocation( "appdata" );
    QCOMPARE( fooAppData, m_kdehome + "/share/apps/foo/" );
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
#define KIOSLAVE "bin/kioslave.exe"
#else
#define EXT ""
#define KIOSLAVE "kde4/libexec/kioslave"
#endif
    const QString bin = KGlobal::dirs()->findResource( "exe", "kioslave" EXT );
    QVERIFY( !bin.isEmpty() );
    QVERIFY( bin.endsWith( KIOSLAVE ) );
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
    const QString kdeinit = KGlobal::dirs()->findExe( "kdeinit4" );
    QVERIFY( !kdeinit.isEmpty() );
    QVERIFY( kdeinit.endsWith( "bin/kdeinit4" EXT ) );

#ifdef Q_OS_UNIX
    // findExe with a result in libexec
    const QString lnusertemp = KGlobal::dirs()->findExe( "lnusertemp" );
    QVERIFY( !lnusertemp.isEmpty() );
    QVERIFY( lnusertemp.endsWith( "lib" KDELIBSUFF "/kde4/libexec/lnusertemp" EXT ) );
#endif

    // Check the "exe" resource too
    QString kdeinitPath1 = KGlobal::dirs()->realFilePath(kdeinit);
    QString kdeinitPath2 = KGlobal::dirs()->locate( "exe", "kdeinit4" );
#ifdef Q_WS_WIN // one path can be c:/... and second C:/...
    kdeinitPath1 = kdeinitPath1.toLower();
    kdeinitPath2 = kdeinitPath2.toLower();
#endif
    QCOMPARE( kdeinitPath1, kdeinitPath2 );

#ifdef Q_OS_UNIX
    QCOMPARE( KGlobal::dirs()->realFilePath(lnusertemp),
              KGlobal::dirs()->locate( "exe", "lnusertemp" ) );

    // findExe with relative path
    const QString pwd = QDir::currentPath();
    QDir::setCurrent("/bin");
    QCOMPARE(KGlobal::dirs()->findExe("./sh"), QString::fromLatin1("/bin/sh"));
    QDir::setCurrent(pwd);
#endif

#ifdef Q_OS_UNIX
    QFile home( m_kdehome );
    const QString target = m_kdehome + "/linktodir";
    home.link( target );
    QVERIFY( KGlobal::dirs()->findExe( target ).isEmpty() );
#endif

#ifdef Q_OS_UNIX
    // findExe for a binary not part of KDE
    const QString ls = KGlobal::dirs()->findExe( "ls" );
    QVERIFY( !ls.isEmpty() );
    QVERIFY( ls.endsWith( "bin/ls" ) );
#endif

    // findExe with no result
    const QString idontexist = KGlobal::dirs()->findExe( "idontexist" );
    QVERIFY( idontexist.isEmpty() );

    // findExe with empty string
    const QString empty = KGlobal::dirs()->findExe( "" );
    QVERIFY( empty.isEmpty() );
}

void KStandarddirsTest::testLocate()
{
    QString textPlain = "text/x-patch.xml";
    Q_FOREACH( const QString &path, KGlobal::dirs()->resourceDirs("xdgdata-mime") ) {
        if (QFile::exists(path + textPlain)) {
            textPlain = path + textPlain;
            break;
        }
    }
    if( textPlain == "text/x-patch.xml" )
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

    QString ret = KStandardDirs::locate( "dtd", "customization/catalog.xml" );
    QCOMPARE(ret, QString()); // normal, there's no "dtd" resource in kstandarddirs by default

    KGlobal::dirs()->addResourceType("dtd", "data", "ksgmltools2/");
    ret = KStandardDirs::locate( "dtd", "customization/catalog.xml" );
    QVERIFY(!ret.isEmpty());

    ret = KStandardDirs::locate("dtd", "customization/kde-chunk.xsl");
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

void KStandarddirsTest::testSetXdgDataDirs()
{
    // By default we should have KDEDIR/share/applications in `kde4-config --path xdgdata-apps`
    const QStringList dirs = KGlobal::dirs()->resourceDirs("xdgdata-apps");
    const QString kdeDataApps = KStandardDirs::realPath(KDEDIR "/share/applications/");
    QVERIFY(dirs.contains(kdeDataApps));

    // When setting XDG_DATA_DIR this should still be true
    const QString localApps = m_kdehome + "/share/applications/";
    QVERIFY(KStandardDirs::makeDir(localApps));
    ::setenv("XDG_DATA_DIRS", QFile::encodeName(m_kdehome + "/share"), 1 );
    KStandardDirs newStdDirs;
    const QStringList newDirs = newStdDirs.resourceDirs("xdgdata-apps");
    QVERIFY(newDirs.contains(kdeDataApps));
    QVERIFY(newDirs.contains(localApps));
}

void KStandarddirsTest::testRestrictedResources()
{
    // Ensure we have a local xdgdata-apps dir
    QFile localFile(KStandardDirs::locateLocal("xdgdata-apps", "foo.desktop"));
    localFile.open(QIODevice::WriteOnly|QIODevice::Text);
    localFile.write("foo");
    localFile.close();
    const QString localAppsDir = QFileInfo(localFile).absolutePath() + '/';
    QVERIFY(!localAppsDir.contains("foo.desktop"));
    // Ensure we have a local share/apps/qttest dir
    const QString localDataDir = KStandardDirs::locateLocal("data", "qttest/");
    QVERIFY(!localDataDir.isEmpty());
    QVERIFY(QFile::exists(localDataDir));
    const QString localOtherDataDir = KStandardDirs::locateLocal("data", "other/");
    QVERIFY(!localOtherDataDir.isEmpty());

    // Check unrestricted results first
    const QStringList appsDirs = KGlobal::dirs()->resourceDirs("xdgdata-apps");
    const QString kdeDataApps = KStandardDirs::realPath(KDEDIR "/share/applications/");
    QVERIFY(appsDirs.contains(kdeDataApps));
    QVERIFY(appsDirs.contains(localAppsDir));
    const QStringList dataDirs = KGlobal::dirs()->findDirs("data", "qttest");
    QVERIFY(dataDirs.contains(localDataDir));
    const QStringList otherDataDirs = KGlobal::dirs()->findDirs("data", "other");
    QVERIFY(otherDataDirs.contains(localOtherDataDir));

    // Initialize restrictions.
    // Need a new componentdata to trigger restricted-resource initialization
    // And we need to write the config _before_ creating the KComponentData.
    KConfig foorc("foorc");
    KConfigGroup restrictionsGroup(&foorc, "KDE Resource Restrictions");
    restrictionsGroup.writeEntry("xdgdata-apps", false);
    restrictionsGroup.writeEntry("data_qttest", false);
    restrictionsGroup.sync();

    // Check restrictions.
    KComponentData cData("foo");
    QVERIFY(cData.dirs()->isRestrictedResource("xdgdata-apps"));
    QVERIFY(cData.dirs()->isRestrictedResource("data", "qttest"));

    const QStringList newAppsDirs = cData.dirs()->resourceDirs("xdgdata-apps");
    QVERIFY(newAppsDirs.contains(kdeDataApps));
    QVERIFY(!newAppsDirs.contains(localAppsDir)); // restricted!
    const QStringList newDataDirs = cData.dirs()->findDirs("data", "qttest");
    QVERIFY(!newDataDirs.contains(localDataDir)); // restricted!
    const QStringList newOtherDataDirs = cData.dirs()->findDirs("data", "other");
    QVERIFY(newOtherDataDirs.contains(localOtherDataDir)); // not restricted!

    restrictionsGroup.deleteGroup();
}

#include <QThreadPool>
#include <qtconcurrentrun.h>

// To find multithreading bugs: valgrind --tool=helgrind ./kstandarddirstest testThreads
void KStandarddirsTest::testThreads()
{
    QThreadPool::globalInstance()->setMaxThreadCount(5);
    QFuture<void> f1 = QtConcurrent::run(this, &KStandarddirsTest::testLocateLocal);
    QFuture<void> f2 = QtConcurrent::run(this, &KStandarddirsTest::testSaveLocation);
    QFuture<void> f3 = QtConcurrent::run(this, &KStandarddirsTest::testFindResource);
    QFuture<void> f4 = QtConcurrent::run(this, &KStandarddirsTest::testFindAllResources);
    f1.waitForFinished();
    f2.waitForFinished();
    f3.waitForFinished();
    f4.waitForFinished();
}
