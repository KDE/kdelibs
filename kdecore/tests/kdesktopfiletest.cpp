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
#include "kdesktopfiletest.h"
#include <kconfiggroup.h>
#include <ktemporaryfile.h>
#include <kstandarddirs.h>
#include "kdesktopfiletest.moc"

#include "kdesktopfile.h"

#include <qtest_kde.h>

QTEST_KDEMAIN_CORE( KDesktopFileTest )

void KDesktopFileTest::testRead()
{
    const QString fileName = QFile::decodeName(KDESRCDIR "/../services/kplugininfo.desktop");
    QVERIFY(QFile::exists(fileName));
    QVERIFY(KDesktopFile::isDesktopFile(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.readType(), QString::fromLatin1("ServiceType"));
    QCOMPARE(df.readIcon(), QString());
    QCOMPARE(df.readName(), QString::fromLatin1("KDE Plugin Information"));
    QCOMPARE(df.hasLinkType(), false);
    QCOMPARE(df.hasMimeTypeType(), false);
    QCOMPARE(df.hasApplicationType(), false);
    QCOMPARE(df.fileName(), fileName);
    QCOMPARE(df.resource(), "apps"); // default for .desktop files
}

void KDesktopFileTest::testActionGroup()
{
    KTemporaryFile file;
    file.setPrefix("test1");
    QVERIFY( file.open() );
    const QString fileName = file.fileName();
    QTextStream ts( &file );
    ts <<
        "[Desktop Entry]\n"
        "Actions=encrypt;\n"
        "[Desktop Action encrypt]\n"
        "Name=Encrypt file\n"
        "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.readType(), QString());
    QCOMPARE(df.fileName(), fileName);
    QCOMPARE(df.readActions(), QStringList() << "encrypt");
    QCOMPARE(df.hasActionGroup("encrypt"), true);
    QCOMPARE(df.hasActionGroup("doesnotexist"), false);
    KConfigGroup cg = df.actionGroup("encrypt");
    QVERIFY(cg.hasKey("Name"));
    QCOMPARE(cg.readEntry("Name"), QString("Encrypt file"));
}

void KDesktopFileTest::testIsAuthorizedDesktopFile()
{
    const QString fileName = QFile::decodeName(KDESRCDIR "../../kioslave/http/http_cache_cleaner.desktop");
    QVERIFY(QFile::exists(fileName));
    QVERIFY(!KDesktopFile::isAuthorizedDesktopFile(fileName));

    const QString installedFile = KGlobal::dirs()->locate("services", "http_cache_cleaner.desktop");
    if (!installedFile.isEmpty()) {
        QVERIFY(KDesktopFile::isAuthorizedDesktopFile(installedFile));
    } else {
        qWarning("Skipping test for http_cache_cleaner.desktop, not found. kdelibs not installed?");
    }

    const QString autostartFile = KStandardDirs::locate("autostart", "plasma-desktop.desktop");
    if (!autostartFile.isEmpty()) {
        QVERIFY(KDesktopFile::isAuthorizedDesktopFile(autostartFile));
    } else {
        qWarning("Skipping test for plasma-desktop.desktop, not found. kdebase not installed?");
    }

}
