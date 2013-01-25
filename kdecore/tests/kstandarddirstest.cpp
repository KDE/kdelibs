/* This file is part of the KDE libraries
    Copyright (c) 2006, 2011 David Faure <faure@kde.org>

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

#include "kstandarddirstest.h"

#include "qtest_kde.h"
#include "kstandarddirstest.moc"

QTEST_KDEMAIN_CORE( KStandarddirsTest )

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kglobal.h>
#include <ktempdir.h>
#include <config-prefix.h>
#include <QtCore/QDebug>
#include <kconfiggroup.h>
#include <config.h>

// we need case-insensitive comparison of file paths on windows
#ifdef Q_OS_WIN
#define QCOMPARE_PATHS(x,y) QCOMPARE(QString(x).toLower(), QString(y).toLower())
#define PATH_SENSITIVITY Qt::CaseInsensitive
#else
#define QCOMPARE_PATHS(x,y) QCOMPARE(QString(x), QString(y))
#define PATH_SENSITIVITY Qt::CaseSensitive
#endif

void KStandarddirsTest::initTestCase()
{
    m_kdehome = QDir::home().canonicalPath() + "/.kde-unit-test";
}

void KStandarddirsTest::testLocateLocal()
{
    const QString configLocal = KStandardDirs::locateLocal( "config", "ksomethingrc" );
    // KStandardDirs resolves symlinks, so we must compare with canonicalPath()
    QCOMPARE_PATHS( configLocal, m_kdehome + "/share/config/ksomethingrc" );
}

void KStandarddirsTest::testSaveLocation()
{
    const QString saveLoc = KGlobal::dirs()->saveLocation( "appdata" );
    QCOMPARE_PATHS( saveLoc, m_kdehome + "/share/apps/qttest/" );
}

void KStandarddirsTest::testAppData()
{
    // In addition to testSaveLocation(), we want to also check other KComponentDatas
    KComponentData cData("foo");
    const QString fooAppData = cData.dirs()->saveLocation( "appdata" );
    QCOMPARE_PATHS( fooAppData, m_kdehome + "/share/apps/foo/" );
}

void KStandarddirsTest::testChangeSaveLocation()
{
    KComponentData cData("changeSave");
    QCOMPARE_PATHS(cData.dirs()->saveLocation("config"), m_kdehome + "/share/config/");
    // Can we change the save location?
    const QString newSaveLoc = m_kdehome + "/newconfigdir/";
    //cData.dirs()->addResourceDir("config", newSaveLoc); // can't be done, absolute paths have less priority than relative paths
    cData.dirs()->addResourceType("config", 0, "newconfigdir");
    QCOMPARE_PATHS(KStandardDirs::realPath(cData.dirs()->saveLocation("config")), newSaveLoc);
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

    const QString data = KGlobal::dirs()->findResource( "data", "cmake/modules/FindSoprano.cmake" );
    QVERIFY( !data.isEmpty() );
    QVERIFY( data.endsWith( QLatin1String("share/apps/cmake/modules/FindSoprano.cmake") ) );
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

    const QStringList cmakeModulesFiles = KGlobal::dirs()->findAllResources( "data", "cmake/modules/" );
    QVERIFY( !cmakeModulesFiles.isEmpty() );
    QVERIFY( cmakeModulesFiles.count() > 80 ); // I have 150 here, installed by kdelibs.

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
    QVERIFY( oneEndsWith( configFilesRecursive, "share/config/ui/ui_standards.rc" ) );
    QVERIFY( oneEndsWith( configFilesRecursive, "share/config/colors/Web.colors" ) ); // proves that recursive worked

    const QStringList configFilesRecursiveWithFilter = KGlobal::dirs()->findAllResources( "config", "*rc",
                                                                                          KStandardDirs::Recursive );
    QVERIFY( !configFilesRecursiveWithFilter.isEmpty() );
    QVERIFY( configFilesRecursiveWithFilter.count() >= 4 );
    QVERIFY( oneEndsWith( configFilesRecursiveWithFilter, "share/config/kdebugrc" ) );
    QVERIFY( oneEndsWith( configFilesRecursiveWithFilter, "share/config/ui/ui_standards.rc" ) );
    QVERIFY( !oneEndsWith( configFilesRecursiveWithFilter, "share/config/colors/Web.colors" ) ); // didn't match the filter

    QStringList fileNames;
    const QStringList configFilesWithFilter = KGlobal::dirs()->findAllResources("config", "*rc", KStandardDirs::NoDuplicates, fileNames);
    QVERIFY( !configFilesWithFilter.isEmpty() );
    QVERIFY( oneEndsWith( configFilesWithFilter, "share/config/kdebugrc" ) );
    QVERIFY2( configFilesWithFilter.count() >= 4, qPrintable(configFilesWithFilter.join(",")) );
    QVERIFY( !oneEndsWith( configFilesWithFilter, "share/config/ui/ui_standards.rc" ) ); // not recursive
    QVERIFY( !oneEndsWith( configFilesWithFilter, "share/config/accept-languages.codes" ) ); // didn't match the filter
    QCOMPARE(fileNames.count(), configFilesWithFilter.count());
    QVERIFY(fileNames.contains("kdebugrc"));

#if 0
    list = t.findAllResources("html", "en/*/index.html", false);
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
        kDebug() << "docs " << (*it).toLatin1().constData();
    }

    list = t.findAllResources("html", "*/*/*.html", false);
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
        kDebug() << "docs " << (*it).toLatin1().constData();
    }
#endif
}

void KStandarddirsTest::testFindAllResourcesNewDir()
{
    const QStringList origFiles = KGlobal::dirs()->findAllResources("data", "cmake/modules/");
    const int origCount = origFiles.count();

    const QString dir = m_kdehome + "/share/apps/cmake/modules";
    QDir().mkpath(dir);
    QFile file(dir+"/unittest.testfile");
    QVERIFY(file.open(QIODevice::WriteOnly|QIODevice::Text));
    file.write("foo");
    file.close();

    const int newCount = KGlobal::dirs()->findAllResources("data", "cmake/modules/").count();
    QCOMPARE(newCount, origCount+1);
    file.remove();
    QDir().rmpath(dir);
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
    QVERIFY( configDir.endsWith( QLatin1String( "/config/" ) ) );
}

void KStandarddirsTest::testFindExe()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed", SkipAll );

    // findExe with a result in bin
    const QString kdeinit = KGlobal::dirs()->findExe( "kdeinit4" );
    QVERIFY( !kdeinit.isEmpty() );
#ifdef Q_OS_MAC
    QVERIFY2(kdeinit.endsWith("kdeinit4", PATH_SENSITIVITY), qPrintable(kdeinit));
#else
    QVERIFY2(kdeinit.endsWith("bin/kdeinit4" EXT, PATH_SENSITIVITY), qPrintable(kdeinit));
#endif


#ifdef Q_OS_UNIX
    // findExe with a result in libexec
    const QString lnusertemp = KGlobal::dirs()->findExe( "lnusertemp" );
    QVERIFY( !lnusertemp.isEmpty() );
    QVERIFY( lnusertemp.endsWith( "lib" KDELIBSUFF "/kde4/libexec/lnusertemp" EXT, PATH_SENSITIVITY ) );
#endif

#ifndef Q_OS_MAC // kdeinit4 is a bundle on Mac, so the below doesn't work
    // Check the "exe" resource too
    QString kdeinitPath1 = KGlobal::dirs()->realFilePath(kdeinit);
    QString kdeinitPath2 = KGlobal::dirs()->locate( "exe", "kdeinit4" );
    QCOMPARE_PATHS( kdeinitPath1, kdeinitPath2 );
#endif

#ifdef Q_OS_UNIX
    QCOMPARE_PATHS( KGlobal::dirs()->realFilePath(lnusertemp),
              KGlobal::dirs()->locate( "exe", "lnusertemp" ) );

    // findExe with relative path
    const QString pwd = QDir::currentPath();
    QDir::setCurrent("/bin");
    QStringList possibleResults;
    possibleResults << QString::fromLatin1("/bin/sh") << QString::fromLatin1("/usr/bin/sh");
    const QString sh = KGlobal::dirs()->findExe("./sh");
    if (!possibleResults.contains(sh)) {
        kDebug() << sh;
    }
    QVERIFY(possibleResults.contains(sh));
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
    QVERIFY( ls.endsWith( QLatin1String( "bin/ls" ) ) );
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
    QCOMPARE_PATHS(res, textPlain);
}

void KStandarddirsTest::testRelativeLocation()
{
    const QString file = "kdebugrc";
    QString located = KGlobal::dirs()->locate( "config", file );
    QCOMPARE_PATHS( KGlobal::dirs()->relativeLocation( "config", located ), file );
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

    const QStringList files = KGlobal::dirs()->findAllResources("dtd", "customization/*", KStandardDirs::NoDuplicates);
    QVERIFY(files.count() > 2);

    KGlobal::dirs()->addResourceType("xdgdata-ontology", 0, "ontology");
    const QStringList ontologyDirs = KGlobal::dirs()->resourceDirs("xdgdata-ontology");
    QCOMPARE(ontologyDirs.first(), KStandardDirs::realPath(QString(qgetenv("XDG_DATA_HOME")) + "/ontology/"));
    if (QFile::exists("/usr/share/ontology") &&
        KGlobal::dirs()->kfsstnd_xdg_data_prefixes().contains("/usr/share")) {
        QVERIFY(ontologyDirs.contains("/usr/share/ontology/"));
    }
}

void KStandarddirsTest::testAddResourceDir()
{
    const QString dir = QString::fromLatin1(KDESRCDIR);
    const QString file = "Cairo";
    QString ret = KStandardDirs::locate( "here", file );
    QCOMPARE(ret, QString()); // not set up yet

    KGlobal::dirs()->addResourceDir("here", dir);
    ret = KStandardDirs::locate( "here", file );
    QCOMPARE_PATHS(ret, KStandardDirs::realPath(dir) + "Cairo");
}

void KStandarddirsTest::testSetXdgDataDirs()
{
    // By default we should have KDEDIR/share/applications in `kde4-config --path xdgdata-apps`
    const QStringList dirs = KGlobal::dirs()->resourceDirs("xdgdata-apps");
    const QString kdeDataApps = KStandardDirs::realPath(KDEDIR "/share/applications/");
    if (!dirs.contains(kdeDataApps)) {
        kDebug() << "ERROR:" << kdeDataApps << "not in" << dirs;
        kDebug() << "XDG_DATA_DIRS=" << qgetenv("XDG_DATA_DIRS");
        kDebug() << "installprefix=" << KStandardDirs::installPath("kdedir");
        kDebug() << "installdir=" << KStandardDirs::installPath("xdgdata-apps");
        kDebug() << "KStandardDirs::kfsstnd_xdg_data_prefixes=" << KGlobal::dirs()->kfsstnd_xdg_data_prefixes();
    }
    QVERIFY(dirs.contains(kdeDataApps, PATH_SENSITIVITY));

    // When setting XDG_DATA_DIR this should still be true
    const QString localApps = m_kdehome + "/share/applications/";
    QVERIFY(KStandardDirs::makeDir(localApps));
    ::setenv("XDG_DATA_DIRS", QFile::encodeName(m_kdehome + "/share"), 1 );
    KStandardDirs newStdDirs;
    const QStringList newDirs = newStdDirs.resourceDirs("xdgdata-apps");
    QVERIFY(newDirs.contains(kdeDataApps, PATH_SENSITIVITY));
    QVERIFY(newDirs.contains(localApps, PATH_SENSITIVITY));
}

void KStandarddirsTest::testRestrictedResources()
{
    // Ensure we have a local xdgdata-apps dir
    QFile localFile(KStandardDirs::locateLocal("xdgdata-apps", "foo.desktop"));
    localFile.open(QIODevice::WriteOnly|QIODevice::Text);
    localFile.write("foo");
    localFile.close();
    const QString localAppsDir = KStandardDirs::realPath(QFileInfo(localFile).absolutePath() + '/');
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
    QCOMPARE_PATHS(appsDirs.first(), localAppsDir);
    QVERIFY(appsDirs.contains(kdeDataApps, PATH_SENSITIVITY));
    const QStringList dataDirs = KGlobal::dirs()->findDirs("data", "qttest");
    QCOMPARE_PATHS(dataDirs.first(), localDataDir);
    const QStringList otherDataDirs = KGlobal::dirs()->findDirs("data", "other");
    QCOMPARE_PATHS(otherDataDirs.first(), localOtherDataDir);

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
    QVERIFY(newAppsDirs.contains(kdeDataApps, PATH_SENSITIVITY));
    QVERIFY(!newAppsDirs.contains(localAppsDir, PATH_SENSITIVITY)); // restricted!
    const QStringList newDataDirs = cData.dirs()->findDirs("data", "qttest");
    QVERIFY(!newDataDirs.contains(localDataDir, PATH_SENSITIVITY)); // restricted!
    const QStringList newOtherDataDirs = cData.dirs()->findDirs("data", "other");
    QVERIFY(newOtherDataDirs.contains(localOtherDataDir, PATH_SENSITIVITY)); // not restricted!

    restrictionsGroup.deleteGroup();
    localFile.remove();
}

void KStandarddirsTest::testSymlinkResolution()
{
#ifndef Q_OS_WIN
    // This makes the save location for the david resource, "$HOME/.kde-unit-test/symlink/test/"
    // where symlink points to "real", and the subdir test will be created later
    // This used to confuse KStandardDirs and make it return unresolved paths,
    // and thus making comparisons fail later on in KConfig.
    const QString symlink = m_kdehome + "/symlink";
    const QString expected = m_kdehome + "/real/test/";
    QVERIFY(KTempDir::removeDir(m_kdehome + "/real"));
    QVERIFY(QDir(m_kdehome).mkdir("real"));
    QFile::remove(symlink);
    QVERIFY(!QFile::exists(symlink));
    QVERIFY(QFile::link("real", symlink));
    QVERIFY(QFileInfo(symlink).isSymLink());
    QVERIFY(!QFile::exists(expected));
    KGlobal::dirs()->addResourceType("david", 0, "symlink/test");
    QVERIFY(!QFile::exists(expected));
    const QString saveLoc = KGlobal::dirs()->resourceDirs("david").first();
    QVERIFY(!QFile::exists(expected));
    // The issue at this point is that saveLoc does not actually exist yet.
    QVERIFY(QDir(saveLoc).canonicalPath().isEmpty()); // this is why we can't use canonicalPath
    QVERIFY(!QFile::exists(saveLoc));
    QCOMPARE(saveLoc, KStandardDirs::realPath(saveLoc)); // must be resolved
    QCOMPARE(saveLoc, expected);
    QVERIFY(QDir(m_kdehome).mkpath("real/test")); // KConfig calls mkdir on its own, we simulate that here
    const QString sameSaveLoc = KGlobal::dirs()->resourceDirs("david").first();
    QCOMPARE(sameSaveLoc, saveLoc);
    QCOMPARE(sameSaveLoc, KGlobal::dirs()->saveLocation("david"));

    // While we're here...
    QCOMPARE(KStandardDirs::realPath(QString()), QString());
    QCOMPARE(KStandardDirs::realPath(QString("/")), QString("/"));

    QCOMPARE(KStandardDirs::realPath(QString("/does_not_exist/")), QString("/does_not_exist/"));
#endif
}

#include <QThreadPool>
#include <qtconcurrentrun.h>

// To find multithreading bugs: valgrind --tool=helgrind ./kstandarddirstest testThreads
void KStandarddirsTest::testThreads()
{
    QThreadPool::globalInstance()->setMaxThreadCount(6);
    QList<QFuture<void> > futures;
    futures << QtConcurrent::run(this, &KStandarddirsTest::testLocateLocal);
    futures << QtConcurrent::run(this, &KStandarddirsTest::testSaveLocation);
    futures << QtConcurrent::run(this, &KStandarddirsTest::testAppData);
    futures << QtConcurrent::run(this, &KStandarddirsTest::testFindResource);
    futures << QtConcurrent::run(this, &KStandarddirsTest::testFindAllResources);
    futures << QtConcurrent::run(this, &KStandarddirsTest::testLocate);
    futures << QtConcurrent::run(this, &KStandarddirsTest::testRelativeLocation);
    Q_FOREACH(QFuture<void> f, futures) // krazy:exclude=foreach
        f.waitForFinished();
}
