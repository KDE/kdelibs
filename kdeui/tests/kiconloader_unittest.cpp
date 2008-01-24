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

#include <kicon.h>
#include "qtest_kde.h"
#include <kiconloader.h>
#include <kstandarddirs.h>

class KIconLoader_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
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
    }

    void testAppPicsDir_KIcon()
    {
        const QString dataDir = KStandardDirs::realPath(KDESRCDIR "/../../");
        KGlobal::dirs()->addResourceDir("data", dataDir);
        const QString appName = "kdewidgets";
        KIconLoader appIconLoader(appName);
        // Now using KIcon. Separate test so that KIconLoader isn't fully inited.
        KIcon icon("kdialog", &appIconLoader);
        QPixmap pix = icon.pixmap(QSize(22, 22));
        QVERIFY(!pix.isNull());
    }
};

QTEST_KDEMAIN(KIconLoader_UnitTest, GUI)

#include "kiconloader_unittest.moc"
