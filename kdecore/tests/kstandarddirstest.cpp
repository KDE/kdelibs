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
#include "qstandardpaths.h"

#include "qtest.h"

QTEST_MAIN(KStandarddirsTest)

#include <config-kstandarddirs.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kconfig.h>
#include <kglobal.h>
#include <qtemporarydir.h>
#include <config-prefix.h>
#include <QtCore/QDebug>
#include <kconfiggroup.h>

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
    m_configHome = QDir::homePath() + QLatin1String("/.kde-unit-test/xdg/config");
    qputenv("XDG_CONFIG_HOME", QFile::encodeName(m_configHome));

    m_dataHome = QDir::homePath() + QLatin1String("/.kde-unit-test/xdg/local");
    qputenv("XDG_DATA_HOME", QFile::encodeName(m_dataHome));

    QFile::remove(KGlobal::dirs()->saveLocation("config") + "kstandarddirstestrc");

    // Create a main component data so that testAppData doesn't suddenly change the main component
    // data.
    KComponentData mainData("kstandarddirstest");

    // Must initialize KStandardDirs only after all the setenv() calls.
    QCOMPARE(KGlobal::dirs()->localxdgconfdir(), QString(m_configHome + '/'));
}

void KStandarddirsTest::testSaveLocation()
{
    const QString saveLocConfig = KGlobal::dirs()->saveLocation("config");
    QCOMPARE_PATHS(saveLocConfig, KGlobal::dirs()->localxdgconfdir());
    const QString saveLocXdgConfig = KGlobal::dirs()->saveLocation("xdgconf");
    QCOMPARE_PATHS(saveLocConfig, saveLocXdgConfig); // same result

    const QString saveLocAppData = KGlobal::dirs()->saveLocation("appdata");
    QCOMPARE_PATHS(saveLocAppData, m_dataHome + "/kstandarddirstest/");
}

void KStandarddirsTest::testLocateLocal()
{
    const QString configLocal = KStandardDirs::locateLocal( "config", "ksomethingrc" );
    // KStandardDirs resolves symlinks, so we must compare with canonicalPath()
    QCOMPARE_PATHS( configLocal, m_configHome + "/ksomethingrc" );
}

void KStandarddirsTest::testResourceDirs()
{
    const QStringList configDirs = KGlobal::dirs()->resourceDirs("config");
    Q_FOREACH(const QString& dir, configDirs) {
        QVERIFY2(dir.endsWith("xdg/")
                 || dir.endsWith("share/config/") // KDE4 compat path
                 || dir.endsWith(".kde-unit-test/xdg/config/"), qPrintable(dir));
    }

    const QStringList dataDirs = KGlobal::dirs()->resourceDirs("data");
    Q_FOREACH(const QString& dir, dataDirs) {
        QVERIFY2(dir.endsWith("share/") || dir.endsWith(".kde-unit-test/xdg/local/"), qPrintable(dir));
    }
}

void KStandarddirsTest::testAppData()
{
    // This API is gone
#if 0
    // In addition to testSaveLocation(), we want to also check other KComponentDatas
    KComponentData cData("foo");
    const QString fooAppData = cData.dirs()->saveLocation( "appdata" );
    QCOMPARE_PATHS( fooAppData, m_dataHome + "/foo/" );
#endif
}

void KStandarddirsTest::testChangeSaveLocation()
{
    KStandardDirs cData;
    QCOMPARE_PATHS(cData.saveLocation("config"), m_configHome + "/");
    // Can we change the save location?
    const QString newSaveLoc = m_configHome + "/newconfigdir/";
    //cData.addResourceDir("config", newSaveLoc); // can't be done, absolute paths have less priority than relative paths
    cData.addResourceType("config", 0, "newconfigdir");
    QCOMPARE_PATHS(KStandardDirs::realPath(cData.saveLocation("config")), newSaveLoc);
}

static bool isKdelibsInstalled()
{
    // If there's only one dir, it's the local one (~/.kde-unit-test/share/),
    // meaning that kdelibs wasn't installed (or we don't find where, the environment isn't right).
    return KGlobal::dirs()->resourceDirs( "data" ).count() > 1;
}

void KStandarddirsTest::testFindResource()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed" );

#ifdef Q_OS_WIN
#define EXT ".exe"
#define KIOSLAVE "bin/kioslave.exe"
#else
#define EXT ""
#define KIOSLAVE "kde5/libexec/kioslave"
#endif
    const QString bin = KGlobal::dirs()->findResource( "exe", "kioslave" EXT );
    QVERIFY( !bin.isEmpty() );
    QVERIFY( bin.endsWith( KIOSLAVE ) );
    QVERIFY( !QDir::isRelativePath(bin) );

    const QString data = KGlobal::dirs()->findResource( "data", "cmake/modules/FindSoprano.cmake" );
    QVERIFY( !data.isEmpty() );
    QVERIFY( data.endsWith( QLatin1String("share/cmake/modules/FindSoprano.cmake") ) );
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
        QSKIP( "kdelibs not installed" );

    const QStringList cmakeModulesFiles = KGlobal::dirs()->findAllResources( "data", "cmake/modules/" );
    QVERIFY( !cmakeModulesFiles.isEmpty() );
    QVERIFY( cmakeModulesFiles.count() > 80 ); // I have 150 here, installed by kdelibs.

    // Create a local config file, the file will be used as expected result
    KConfig foorc("foorc");
    KConfigGroup dummyGroup(&foorc, "Dummy");
    dummyGroup.writeEntry("someEntry", true);
    dummyGroup.sync();
    const QString localConfigFile = KGlobal::dirs()->localxdgconfdir() + "foorc";
    QVERIFY2(QFile::exists(localConfigFile), qPrintable(localConfigFile));

    const QStringList configFiles = KGlobal::dirs()->findAllResources( "config" );
    QVERIFY( !configFiles.isEmpty() );
    QVERIFY( configFiles.count() > 5 ); // I have 9 here
    QVERIFY( oneEndsWith( configFiles, "etc/xdg/kdebugrc" ) );
    QVERIFY( oneEndsWith( configFiles, "kde-unit-test/xdg/config/foorc" ) );
    QVERIFY( !oneEndsWith( configFiles, "etc/xdg/colors/Web.colors" ) ); // recursive was false

    {
    const QStringList configFilesRecursive = KGlobal::dirs()->findAllResources( "config", QString(),
                                                                                KStandardDirs::Recursive );
    QVERIFY( !configFilesRecursive.isEmpty() );
    QVERIFY( configFilesRecursive.count() > 5 ); // I have 15 here
    QVERIFY( oneEndsWith( configFilesRecursive, "etc/xdg/kdebugrc" ) );
    QVERIFY( oneEndsWith( configFilesRecursive, "etc/xdg/colors/Web.colors" ) ); // proves that recursive worked
    }

    {
    const QStringList configFilesRecursiveWithFilter = KGlobal::dirs()->findAllResources( "config", "*rc",
                                                                                          KStandardDirs::Recursive );
    QVERIFY( !configFilesRecursiveWithFilter.isEmpty() );
    //qDebug() << configFilesRecursiveWithFilter;
    QVERIFY( configFilesRecursiveWithFilter.count() >= 3 ); // foorc, kdebugrc, ui/ui_standards.rc
    QVERIFY( oneEndsWith( configFilesRecursiveWithFilter, "kde-unit-test/xdg/config/foorc" ) );
    QVERIFY( oneEndsWith( configFilesRecursiveWithFilter, "etc/xdg/kdebugrc" ) );
    QVERIFY( oneEndsWith( configFilesRecursiveWithFilter, "etc/xdg/ui/ui_standards.rc" ) );
    QVERIFY( !oneEndsWith( configFilesRecursiveWithFilter, "etc/xdg/colors/Web.colors" ) ); // didn't match the filter
    }

    {
    QStringList fileNames;
    const QStringList configFilesWithFilter = KGlobal::dirs()->findAllResources("config", "*rc", KStandardDirs::NoDuplicates, fileNames);
    QVERIFY( !configFilesWithFilter.isEmpty() );
    QVERIFY2( configFilesWithFilter.count() >= 2, qPrintable(configFilesWithFilter.join(",")) );
    QVERIFY( oneEndsWith( configFilesWithFilter, "kde-unit-test/xdg/config/foorc" ) );
    QVERIFY( oneEndsWith( configFilesWithFilter, "kdebugrc" ) ); // either global (etc/xdg/) or local (XDG_HOME)
    QVERIFY( !oneEndsWith( configFilesWithFilter, "etc/xdg/ui/ui_standards.rc" ) ); // recursive not set
    QVERIFY( !oneEndsWith( configFilesWithFilter, "etc/xdg/accept-languages.codes" ) ); // didn't match the filter
    QCOMPARE(fileNames.count(), configFilesWithFilter.count());
    QVERIFY(fileNames.contains("kdebugrc"));
    }

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
    const QString dir = m_dataHome + "/cmake/modules";
    const QString fileName = dir + "/unittest.testfile";
    QFile::remove(fileName);
    const QStringList origFiles = KGlobal::dirs()->findAllResources("data", "cmake/modules/");
    const int origCount = origFiles.count();

    QDir().mkpath(dir);
    QFile file(fileName);
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
        QSKIP( "kdelibs not installed" );

    const QString t = KStandardDirs::locateLocal("data", "kconf_update/" );
    QCOMPARE(t, QString(m_dataHome + "/kconf_update/"));
    const QStringList dirs = KGlobal::dirs()->findDirs( "data", "kconf_update" );
    QVERIFY( !dirs.isEmpty() );
    QVERIFY2(dirs.count() >= 2, qPrintable(dirs.join(","))); // at least local and global
    //qDebug() << dirs;
}

void KStandarddirsTest::testFindResourceDir()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed" );

    const QString configDir = KGlobal::dirs()->findResourceDir( "config", "foorc" );
    QVERIFY( !configDir.isEmpty() );
    QVERIFY2(configDir.endsWith(QLatin1String("/xdg/config/")), qPrintable(configDir));
}

void KStandarddirsTest::testFindExe()
{
    if ( !isKdelibsInstalled() )
        QSKIP( "kdelibs not installed" );

    // findExe with a result in bin
    const QString kdeinit = KGlobal::dirs()->findExe( "kdeinit5" );
    QVERIFY( !kdeinit.isEmpty() );
#ifdef Q_OS_MAC
    QVERIFY2(kdeinit.endsWith("kdeinit5", PATH_SENSITIVITY), qPrintable(kdeinit));
#else
    QVERIFY2(kdeinit.endsWith("bin/kdeinit5" EXT, PATH_SENSITIVITY), qPrintable(kdeinit));
#endif


#ifdef Q_OS_UNIX
    // findExe with a result in libexec
    const QString kioslave = KGlobal::dirs()->findExe( "kioslave" );
    QVERIFY( !kioslave.isEmpty() );
    QVERIFY( kioslave.endsWith( "lib" KDELIBSUFF "/kde5/libexec/kioslave" EXT, PATH_SENSITIVITY ) );
#endif

#ifndef Q_OS_MAC // kdeinit5 is a bundle on Mac, so the below doesn't work
    // Check the "exe" resource too
    QString kdeinitPath1 = KStandardDirs::realFilePath(kdeinit);
    QString kdeinitPath2 = KGlobal::dirs()->locate( "exe", "kdeinit5" );
    QCOMPARE_PATHS( kdeinitPath1, kdeinitPath2 );

    // Check realFilePath behavior with complete command lines, like KRun does
    const QString cmd = kdeinit + " -c foo -x bar";
    const QString fromKStdDirs = KStandardDirs::realFilePath(cmd);
    QCOMPARE(fromKStdDirs, cmd);
    const QString fromQFileInfo = QFileInfo(cmd).canonicalFilePath();
    QVERIFY(fromQFileInfo.isEmpty()); // !! different result, since this doesn't exist as a file
#endif

#ifdef Q_OS_UNIX
    QCOMPARE_PATHS(KGlobal::dirs()->realFilePath(kioslave), KGlobal::dirs()->locate("exe", "kioslave"));

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

#if 0 // Broken test, findExe doesn't look in kdehome, but in kdehome/bin (in kde4) and in $PATH.
#ifdef Q_OS_UNIX
    QFile home( m_kdehome );
    const QString target = m_kdehome + "/linktodir";
    home.link( target );
    QVERIFY( KGlobal::dirs()->findExe( target ).isEmpty() );
#endif
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
        QSKIP("xdg-share-mime not installed");

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
        QSKIP( "kdelibs not installed" );

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
        QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).contains("/usr/share")) {
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
    // By default we should have KDEDIR/share/applications in `kde5-config --path xdgdata-apps`
    const QStringList dirs = KGlobal::dirs()->resourceDirs("xdgdata-apps");
    const QString kdeDataApps = KStandardDirs::realPath(CMAKE_INSTALL_PREFIX "/share/applications/");
    if (!dirs.contains(kdeDataApps)) {
        kDebug() << "ERROR:" << kdeDataApps << "not in" << dirs;
        kDebug() << "XDG_DATA_DIRS=" << qgetenv("XDG_DATA_DIRS");
        kDebug() << "installprefix=" << KStandardDirs::installPath("kdedir");
        kDebug() << "installdir=" << KStandardDirs::installPath("xdgdata-apps");
        kDebug() << "GenericDataLocation=" << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    }
    QVERIFY(dirs.contains(kdeDataApps, PATH_SENSITIVITY));

    // When setting XDG_DATA_DIRS this should still be true
    const QString localApps = m_dataHome + "/applications/";
    QVERIFY(KStandardDirs::makeDir(localApps));
    const QString customDataDir = QDir::homePath() + QLatin1String("/.kde-unit-test/xdg/global");
    qputenv("XDG_DATA_DIRS", QFile::encodeName(customDataDir));
    QVERIFY(QDir(customDataDir).mkpath("applications"));
    KStandardDirs newStdDirs;
    const QStringList newDirs = newStdDirs.resourceDirs("xdgdata-apps");
    //qDebug() << newDirs;
    QVERIFY(newDirs.contains(kdeDataApps, PATH_SENSITIVITY));
    QVERIFY(newDirs.contains(localApps, PATH_SENSITIVITY));
    QVERIFY(newDirs.contains(customDataDir + "/applications/", PATH_SENSITIVITY));
}

void KStandarddirsTest::testRestrictedResources()
{
    // Ensure we have a local xdgdata-apps dir
    QFile localFile(KStandardDirs::locateLocal("xdgdata-apps", "foo.desktop"));
    QVERIFY(localFile.open(QIODevice::WriteOnly|QIODevice::Text));
    localFile.write("foo");
    localFile.close();
    const QString localAppsDir = KStandardDirs::realPath(QFileInfo(localFile).absolutePath() + '/');
    QVERIFY(!localAppsDir.contains("foo.desktop"));
    // Ensure we have a local share/kstandarddirstest dir
    const QString localDataDir = KStandardDirs::locateLocal("data", "kstandarddirstest/");
    QVERIFY(!localDataDir.isEmpty());
    QVERIFY(QFile::exists(localDataDir));
    const QString localOtherDataDir = KStandardDirs::locateLocal("data", "other/");
    QVERIFY(!localOtherDataDir.isEmpty());

    // Check unrestricted results first
    const QStringList appsDirs = KGlobal::dirs()->resourceDirs("xdgdata-apps");
    QCOMPARE_PATHS(appsDirs.first(), localAppsDir);
    const QString kdeDataApps = KStandardDirs::realPath(CMAKE_INSTALL_PREFIX "/share/applications/");
    QVERIFY(appsDirs.contains(kdeDataApps, PATH_SENSITIVITY));
    const QStringList dataDirs = KGlobal::dirs()->findDirs("data", "kstandarddirstest");
    QCOMPARE_PATHS(dataDirs.first(), localDataDir);
    const QStringList otherDataDirs = KGlobal::dirs()->findDirs("data", "other");
    QCOMPARE_PATHS(otherDataDirs.first(), localOtherDataDir);

    // Initialize restrictions.
    // Need a new componentdata to trigger restricted-resource initialization
    // And we need to write the config _before_ creating the KStandardDirs.
    KConfig foorc("kstandarddirstestrc");
    KConfigGroup restrictionsGroup(&foorc, "KDE Resource Restrictions");
    restrictionsGroup.writeEntry("xdgdata-apps", false);
    restrictionsGroup.writeEntry("data_kstandarddirstest", false);
    restrictionsGroup.sync();

    // Check restrictions.
    //KComponentData cData("foo");
    KStandardDirs* dirs = new KStandardDirs;
    dirs->addCustomized(&foorc); // like KGlobal::dirs() does
    QVERIFY(dirs->isRestrictedResource("xdgdata-apps"));
    QVERIFY(dirs->isRestrictedResource("data", "kstandarddirstest"));

    const QStringList newAppsDirs = dirs->resourceDirs("xdgdata-apps");
    QVERIFY(newAppsDirs.contains(kdeDataApps, PATH_SENSITIVITY));
    QVERIFY(!newAppsDirs.contains(localAppsDir, PATH_SENSITIVITY)); // restricted!
    const QStringList newDataDirs = dirs->findDirs("data", "kstandarddirstest");
    QVERIFY(!newDataDirs.contains(localDataDir, PATH_SENSITIVITY)); // restricted!
    const QStringList newOtherDataDirs = dirs->findDirs("data", "other");
    QVERIFY(newOtherDataDirs.contains(localOtherDataDir, PATH_SENSITIVITY)); // not restricted!

    restrictionsGroup.deleteGroup();
    localFile.remove();
    delete dirs;
}

void KStandarddirsTest::testSymlinkResolution()
{
#ifndef Q_OS_WIN
    // This makes the save location for the david resource, "<XDG_DATA_HOME>/symlink/test/"
    // where symlink points to "real", and the subdir test will be created later
    // This used to confuse KStandardDirs and make it return unresolved paths,
    // and thus making comparisons fail later on in KConfig.
    QString baseDir = m_dataHome;
    const QString symlink = baseDir + "/symlink";
    const QString expected = baseDir + "/real/test/";
    QDir d_(baseDir + "/real");
    QVERIFY(d_.removeRecursively());
    QVERIFY(QDir(baseDir).mkdir("real"));
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
    QVERIFY(QDir(baseDir).mkpath("real/test")); // KConfig calls mkdir on its own, we simulate that here
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
#include <QFutureSynchronizer>
#include <qtconcurrentrun.h>

// To find multithreading bugs: valgrind --tool=helgrind ./kstandarddirstest testThreads
void KStandarddirsTest::testThreads()
{
    QThreadPool::globalInstance()->setMaxThreadCount(6);
    QFutureSynchronizer<void> sync;
    sync.addFuture(QtConcurrent::run(this, &KStandarddirsTest::testLocateLocal));
    sync.addFuture(QtConcurrent::run(this, &KStandarddirsTest::testSaveLocation));
    sync.addFuture(QtConcurrent::run(this, &KStandarddirsTest::testAppData));
    sync.addFuture(QtConcurrent::run(this, &KStandarddirsTest::testFindResource));
    sync.addFuture(QtConcurrent::run(this, &KStandarddirsTest::testFindAllResources));
    sync.addFuture(QtConcurrent::run(this, &KStandarddirsTest::testLocate));
    sync.addFuture(QtConcurrent::run(this, &KStandarddirsTest::testRelativeLocation));
    sync.waitForFinished();
}
