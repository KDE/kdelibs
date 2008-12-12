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

#include <kdebug.h>
#include <kicon.h>
#include "qtest_kde.h"
#include <kiconloader.h>
#include <kstandarddirs.h>

class KIconLoader_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        // Remove icon cache (from ~/.kde-unit-test)
        const QString indexFile = KGlobal::dirs()->locateLocal("cache", "kpc/kde-icon-cache.index");
        const QString dataFile = KGlobal::dirs()->locateLocal("cache", "kpc/kde-icon-cache.data");
        QFile::remove(indexFile);
        QFile::remove(dataFile);
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
    }

    void testAppPicsDir()
    {
        // So that we don't rely on installed files, add the toplevel of kdelibs
        // as the "data" resource. But if the file is installed, then it will be
        // preferred (because KStandardDirs::resourceDirs() looks at relative paths first)
        // So we have to expect that one -or- the other will be found.
        const QString dataDir = KStandardDirs::realPath(KDESRCDIR "/../../");
        KGlobal::dirs()->addResourceDir("data", dataDir);

        const QString appName = "kdewidgets";
        KIconLoader appIconLoader(appName);
        QString iconPath = appIconLoader.iconPath("kdialog", KIconLoader::User);
        //QCOMPARE(iconPath, dataDir + appName + "/pics/kdialog.png");
        QVERIFY(iconPath.endsWith(appName + "/pics/kdialog.png"));
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

        appIconLoader.addAppDir( "khtml" );
        QString iconPathFail = appIconLoader.iconPath("fail", KIconLoader::User);
        QVERIFY( iconPathFail.endsWith( "khtml/pics/fail.xpm"));
    }

    void testAppPicsDir_KIcon()
    {
        const QString dataDir = KStandardDirs::realPath(KDESRCDIR "/../../");
        KGlobal::dirs()->addResourceDir("data", dataDir);
        // #### This test is broken; it passes even if appName is set to foobar, because
        // KIcon::pixmap returns an unknown icon if it can't find the real icon...
        const QString appName = "kdewidgets";
        KIconLoader appIconLoader(appName);
        // Now using KIcon. Separate test so that KIconLoader isn't fully inited.
        KIcon icon("kdialog", &appIconLoader);
        QPixmap pix = icon.pixmap(QSize(22, 22));
        QVERIFY(!pix.isNull());
    }

    void testLoadMimeTypeIcon_data()
    {
        QTest::addColumn<QString>("iconName");
        QTest::addColumn<QString>("expectedFileName");

        QTest::newRow("existing icon") << "text-plain" << "text-plain.png";
        QTest::newRow("octet-stream icon") << "application-octet-stream" << "application-octet-stream.png";
        QTest::newRow("non-existing icon") << "foo-bar" << "application-octet-stream.png";
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
        QVERIFY(path.endsWith(expectedFileName));
    }
};

QTEST_KDEMAIN(KIconLoader_UnitTest, GUI)

#include "kiconloader_unittest.moc"
