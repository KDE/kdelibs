/* This file is part of the KDE libraries
    Copyright 2008 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qtest_gui.h"
#include <kiconloader.h>
#include <qprocess.h>
#include <qregexp.h>
#include <qstandardpaths.h>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QtTest/QtTest>
#include <qtemporarydir.h>
#include <kpixmapsequence.h>

class KIconLoader_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::enableTestMode(true);
        // Remove icon cache
        const QString cacheFile = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/icon-cache.kcache";
        QFile::remove(cacheFile);

        // Clear SHM cache
        KIconLoader iconLoader;
        iconLoader.newIconLoader();
    }

    void testUnknownIconNotCached()
    {
        // This is a test to ensure that "unknown" icons do not pin themselves
        // in the icon loader. Or in other words, if an "unknown" icon is
        // returned, but the appropriate icon is subsequently installed
        // properly, the next request for that icon should return the new icon
        // instead of the unknown icon.

        // Since we'll need to create an icon we'll need a temporary directory,
        // and we want that established before creating the icon loader.
        QTemporaryDir tempRoot;
        QVERIFY(tempRoot.isValid());
        QString temporaryDir = tempRoot.path() + QLatin1String("/icons/hicolor/22x22/actions");
        QVERIFY(QDir::root().mkpath(temporaryDir));

        // KDE4:
        //QVERIFY(KGlobal::dirs()->addResourceDir("icon", tempRoot.path(), false));
        // KF5-with-qt4-on-unix: (no solution on other platforms)
        qputenv("XDG_DATA_DIRS", (qgetenv("XDG_DATA_DIRS") + ":" + QFile::encodeName(tempRoot.path())));
        // KF5-with-qt5: port to QStandardPaths::enableTestMode

        KIconLoader iconLoader;

        // First find an existing icon. The only ones installed for sure by
        // kdelibs are the kimproxy ones.
        QString loadedIconPath = iconLoader.iconPath(
                QLatin1String("presence_online"),
                KIconLoader::DefaultState,
                false /* Ensure "unknown" icon can't be returned */
            );
        QVERIFY(!loadedIconPath.isEmpty());

        QString nonExistingIconName = QLatin1String("fhqwhgads_homsar");

        // Find a non-existent icon, allowing unknown icon to be returned
        QPixmap nonExistingIcon = iconLoader.loadIcon(
                nonExistingIconName, KIconLoader::Toolbar);
        QCOMPARE(nonExistingIcon.isNull(), false);

        // Install the existing icon by copying.
        QFileInfo existingIconInfo(loadedIconPath);
        QString newIconPath = temporaryDir + QLatin1String("/")
                              + nonExistingIconName + QLatin1String(".png");
        QVERIFY(QFile::copy(loadedIconPath, newIconPath));

        // Verify the icon can now be found.
        QPixmap nowExistingIcon = iconLoader.loadIcon(
                nonExistingIconName, KIconLoader::Toolbar);
        QVERIFY(nowExistingIcon.cacheKey() != nonExistingIcon.cacheKey());
        QCOMPARE(iconLoader.iconPath(nonExistingIconName, KIconLoader::Toolbar),
                newIconPath);
    }

    void testLoadIconCanReturnNull()
    {
        // This is a test for the "canReturnNull" argument of KIconLoader::loadIcon().
        // We try to load an icon that doesn't exist, first with canReturnNull=false (the default)
        // then with canReturnNull=true.
        KIconLoader iconLoader;
        // We expect a warning here... This doesn't work though, due to the extended debug
        //QTest::ignoreMessage(QtWarningMsg, "KIconLoader::loadIcon: No such icon \"this-icon-does-not-exist\"");
        QPixmap pix = iconLoader.loadIcon("this-icon-does-not-exist", KIconLoader::Desktop, 16);
        QVERIFY(!pix.isNull());
        QCOMPARE(pix.size(), QSize(16, 16));
        // Try it again, to see if the cache interfers
        pix = iconLoader.loadIcon("this-icon-does-not-exist", KIconLoader::Desktop, 16);
        QVERIFY(!pix.isNull());
        QCOMPARE(pix.size(), QSize(16, 16));
        // And now set canReturnNull to true
        pix = iconLoader.loadIcon("this-icon-does-not-exist", KIconLoader::Desktop, 16, KIconLoader::DefaultState,
                                  QStringList(), 0, true);
        QVERIFY(pix.isNull());
        // Try getting the "unknown" icon again, to see if the above call didn't put a null icon into the cache...
        pix = iconLoader.loadIcon("this-icon-does-not-exist", KIconLoader::Desktop, 16);
        QVERIFY(!pix.isNull());
        QCOMPARE(pix.size(), QSize(16, 16));

        // Another one, to clear "last" cache
        pix = iconLoader.loadIcon("this-icon-does-not-exist-either", KIconLoader::Desktop, 16);
        QVERIFY(!pix.isNull());
        QCOMPARE(pix.size(), QSize(16, 16));

        // Now load the initial one again - do we get the warning again?
        pix = iconLoader.loadIcon("this-icon-does-not-exist", KIconLoader::Desktop, 16);
        QVERIFY(!pix.isNull());
        QCOMPARE(pix.size(), QSize(16, 16));

        pix = iconLoader.loadIcon("#crazyname", KIconLoader::NoGroup, 1600);
        QVERIFY(!pix.isNull());
        QCOMPARE(pix.size(), QSize(1600, 1600));
    }

    void testAppPicsDir()
    {
        // So that we don't rely on installed files, add the toplevel of kdelibs
        // as the "data" resource. But if the file is installed, then it will be
        // preferred (because KStandardDirs::resourceDirs() looks at relative paths first)
        // So we have to expect that one -or- the other will be found.
        const QString dataDir = QDir(QFINDTESTDATA("../../..")).canonicalPath();

        const QString appName = "kdewidgets";
        KIconLoader appIconLoader(appName, QStringList() << dataDir);
        QString iconPath = appIconLoader.iconPath("kdialog", KIconLoader::User);
        //QCOMPARE(iconPath, dataDir + appName + "/pics/kdialog.png");
        QVERIFY2(iconPath.endsWith(appName + "/pics/kdialog.png"), qPrintable(iconPath));
        QVERIFY(QFile::exists(iconPath));

        // Load it again, to use the "last loaded" cache
        QString iconPath2 = appIconLoader.iconPath("kdialog", KIconLoader::User);
        QCOMPARE(iconPath, iconPath2);
        // Load something else, to clear the "last loaded" cache
        QString iconPathTextEdit = appIconLoader.iconPath("ktextedit", KIconLoader::User);
        QVERIFY(iconPathTextEdit.endsWith(appName + "/pics/ktextedit.png"));
        QVERIFY(QFile::exists(iconPathTextEdit));
        // Now load kdialog again, to use the real kiconcache
        iconPath2 = appIconLoader.iconPath("kdialog", KIconLoader::User);
        QCOMPARE(iconPath, iconPath2);

        appIconLoader.addAppDir("kdewidgets");
        QString iconPathFail = appIconLoader.iconPath("kurllabel", KIconLoader::User);
        QVERIFY( iconPathFail.endsWith( "kdewidgets/pics/kurllabel.png"));
    }

    void testAppPicsDir_KDE_icon()
    {
        const QString dataDir = QDir(QFINDTESTDATA("../../")).canonicalPath();
        // #### This test is broken; it passes even if appName is set to foobar, because
        // QIcon::pixmap returns an unknown icon if it can't find the real icon...
        const QString appName = "kdewidgets";
        KIconLoader appIconLoader(appName, QStringList() << dataDir);
        // Now using KDE::icon. Separate test so that KIconLoader isn't fully inited.
        QIcon icon = KDE::icon("kdialog", &appIconLoader);
        {
            QPixmap pix = icon.pixmap(QSize(22, 22));
            QVERIFY(!pix.isNull());
        }
        QCOMPARE(icon.actualSize(QSize(96, 22)), QSize(22, 22));
        QCOMPARE(icon.actualSize(QSize(22, 96)), QSize(22, 22));
        QCOMPARE(icon.actualSize(QSize(22, 16)), QSize(16, 16));

        // Can we ask for a really small size?
        {
            QPixmap pix8 = icon.pixmap(QSize(8, 8));
            QCOMPARE(pix8.size(), QSize(8, 8));
        }
    }

    void testLoadMimeTypeIcon_data()
    {
        QTest::addColumn<QString>("iconName");
        QTest::addColumn<QString>("expectedFileName");

        QTest::newRow("existing icon") << "text-plain" << "text-plain.png";
        QTest::newRow("octet-stream icon") << "application-octet-stream" << "application-octet-stream.png";
        QTest::newRow("non-existing icon") << "foo-bar" << "application-octet-stream.png";
        // Test this again, because now we won't go into the "fast path" of loadMimeTypeIcon anymore.
        QTest::newRow("existing icon again") << "text-plain" << "text-plain.png";
        QTest::newRow("generic fallback") << "image-foo-bar" << "image-x-generic.png";
        QTest::newRow("video generic fallback") << "video-foo-bar" << "video-x-generic.png";
        QTest::newRow("image-x-generic itself") << "image-x-generic" << "image-x-generic.png";
        QTest::newRow("x-office-document icon") << "x-office-document" << "x-office-document.png";
        QTest::newRow("unavailable generic icon") << "application/x-font-vfont" << "application-octet-stream.png";
        QTest::newRow("#184852") << "audio/x-tuxguitar" << "audio-x-generic.png";
        QTest::newRow("#178847") << "image/x-compressed-xcf" << "image-x-generic.png";
        QTest::newRow("mimetype generic icon") << "application-x-fluid" << "x-office-document.png";
    }

    void testLoadMimeTypeIcon()
    {
        QFETCH(QString, iconName);
        QFETCH(QString, expectedFileName);
        KIconLoader iconLoader;
        QString path;
        QPixmap pix = iconLoader.loadMimeTypeIcon(iconName, KIconLoader::Desktop, 24,
                                                  KIconLoader::DefaultState, QStringList(),
                                                  &path );
        QVERIFY(!pix.isNull());
        QCOMPARE(path.section('/', -1), expectedFileName);

        // do the same test using a global iconloader, so that
        // we get into the final return statement, which can only happen
        // if d->extraDesktopIconsLoaded becomes true first....
        QString path2;
        pix = KIconLoader::global()->loadMimeTypeIcon(iconName, KIconLoader::Desktop, 24,
                                                      KIconLoader::DefaultState, QStringList(),
                                                      &path2 );
        QVERIFY(!pix.isNull());
        QCOMPARE(path2, path);
    }

    void testPathStore()
    {
        QString path;
        KIconLoader::global()->loadIcon("kde", KIconLoader::Desktop, 24,
                                        KIconLoader::DefaultState, QStringList(),
                                        &path);
        QVERIFY(!path.isEmpty());

        KIconLoader::global()->loadIcon("does_not_exist", KIconLoader::Desktop, 24,
                                        KIconLoader::DefaultState, QStringList(),
                                        &path, true /* canReturnNull */);
        QVERIFY(path.isEmpty());

        path= "some filler to check loadIcon() clears the variable";
        KIconLoader::global()->loadIcon("does_not_exist", KIconLoader::Desktop, 24,
                                        KIconLoader::DefaultState, QStringList(),
                                        &path, true /* canReturnNull */);
        QVERIFY(path.isEmpty());
    }

    void testLoadIconNoGroupOrSize() // #246016
    {
        QPixmap pix = KIconLoader::global()->loadIcon("connected", KIconLoader::NoGroup);
        QVERIFY(!pix.isNull());
    }

    void testLoadPixmapSequence()
    {
       KPixmapSequence seq =  KIconLoader::global()->loadPixmapSequence("process-working", 22);
       QVERIFY(seq.isValid());
    }
};

QTEST_MAIN(KIconLoader_UnitTest)

#include "kiconloader_unittest.moc"
