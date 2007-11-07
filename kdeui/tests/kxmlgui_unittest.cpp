/* This file is part of the KDE libraries

    Copyright (c) 2007 David Faure <faure@kde.org>

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

#include "qtest_kde.h"
#include "kxmlgui_unittest.h"
#include <ktemporaryfile.h>
#include "kxmlgui_unittest.moc"
#include <kxmlguiversionhandler_p.h>
#include <kxmlguiversionhandler.cpp> // it's not exported, so we need to include the code here

QTEST_KDEMAIN(KXmlGui_UnitTest, NoGUI)

enum Flags {
    NoFlags = 0,
    AddToolBars = 1,
    AddModifiedToolBars = 2,
    AddActionProperties = 4,
    AddModifiedMenus = 8
    // next item is 16
};

static void createXmlFile(QFile& file, int version, int flags)
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"" + QByteArray::number(version) + "\" name=\"foo\" >\n"
        "<MenuBar>\n";
    file.write(xml);
    if (flags & AddModifiedMenus) {
        file.write(
            "<Menu noMerge=\"1\" name=\"file\" >\n"
            "<text>&amp;File</text>\n"
            "<Action name=\"file_open\" />\n"
            "</Menu>\n"
            );
    }
    file.write("</MenuBar>\n");
    if (flags & AddToolBars) {
        file.write(
            "<ToolBar name=\"mainToolBar\" fullWidth=\"true\" >\n"
            "  <text>Main Toolbar</text>\n"
            "  <Action name=\"print\" />\n"
            "</ToolBar>\n"
            "<ToolBar name=\"bookmarkToolBar\" fullWidth=\"true\" >\n"
            "  <text>Bookmark Toolbar</text>\n"
            "</ToolBar>\n"
            );
    }
    if (flags & AddModifiedToolBars) {
        file.write(
            "<ToolBar name=\"mainToolBar\" fullWidth=\"true\" >\n"
            "  <text>Main Toolbar</text>\n"
            "  <Action name=\"home\" />\n"
            "</ToolBar>\n"
            "<ToolBar name=\"bookmarkToolBar\" fullWidth=\"true\" >\n"
            "  <text>Modified toolbars</text>\n"
            "</ToolBar>\n"
            );
    }
    if (flags & AddActionProperties) {
        file.write(
            "<ActionProperties>\n"
            "  <Action shortcut=\"F9\" name=\"konq_sidebartng\" />\n"
            "</ActionProperties>\n"
            );
    }
    file.write("</gui>\n");
}

void KXmlGui_UnitTest::testVersionHandlerSameVersion()
{
    // This emulates the case where the user has modified stuff locally
    // and the application hasn't changed since, so the version number is unchanged.
    KTemporaryFile userFile;
    QVERIFY(userFile.open());
    createXmlFile(userFile, 2, AddActionProperties | AddModifiedToolBars);
    const QString firstFile = userFile.fileName();

    KTemporaryFile appFile;
    QVERIFY(appFile.open());
    createXmlFile(appFile, 2, AddToolBars);
    const QString secondFile = appFile.fileName();

    QStringList files;
    files << firstFile << secondFile;

    userFile.close();
    appFile.close();

    KXmlGuiVersionHandler versionHandler(files);
    QCOMPARE(versionHandler.finalFile(), firstFile);
    QString finalDoc = versionHandler.finalDocument();
    QVERIFY(finalDoc.startsWith("<?xml"));
    // Check that the shortcuts defined by the user were kept
    QVERIFY(finalDoc.contains("<ActionProperties>"));
    QVERIFY(finalDoc.contains("sidebartng"));
    // Check that the toolbars modified by the user were kept
    QVERIFY(finalDoc.contains("<Action name=\"home\""));

    QVERIFY(userFile.open());
    const QString userFileContents = QString::fromUtf8(userFile.readAll());
    QCOMPARE(finalDoc, userFileContents);
}

void KXmlGui_UnitTest::testVersionHandlerNewVersionNothingKept()
{
    // This emulates the case where the application has been upgraded
    // and the user has a local ui.rc file, but without shortcuts or toolbar changes.
    // Not sure how this can happen - would be a menu-only app and hand editing !?
    // Anyway the point is to test version number comparison :)

    QMap<QString, int> fileToVersionMap; // makes QCOMPARE failures more readable than just temp filenames

    QFile fileV2(KStandardDirs::locateLocal("appdata", "testui.rc"));
    QVERIFY(fileV2.open(QIODevice::WriteOnly));
    createXmlFile(fileV2, 2, NoFlags);
    fileToVersionMap.insert(fileV2.fileName(), 2);

    KTemporaryFile fileV5;
    QVERIFY(fileV5.open());
    createXmlFile(fileV5, 5, NoFlags);
    fileToVersionMap.insert(fileV5.fileName(), 5);

    // The highest version is neither the first nor last one in the list,
    // to make sure the code really selects the highest version, not just by chance :)
    // (This is why we add the v1 version at the end of the list)
    KTemporaryFile fileV1;
    QVERIFY(fileV1.open());
    createXmlFile(fileV1, 1, NoFlags);
    fileToVersionMap.insert(fileV1.fileName(), 1);


    QStringList files;
    files << fileV2.fileName() << fileV5.fileName() << fileV1.fileName();

    fileV2.close();
    fileV5.close();
    fileV1.close();

    KXmlGuiVersionHandler versionHandler(files);
    QCOMPARE(fileToVersionMap.value(versionHandler.finalFile()), 5);
    QString finalDoc = versionHandler.finalDocument();
    QVERIFY(finalDoc.startsWith("<?xml"));
    QVERIFY(finalDoc.contains("version=\"5\""));

    QVERIFY(fileV5.open());
    const QString fileV5Contents = QString::fromUtf8(fileV5.readAll());
    QCOMPARE(finalDoc, fileV5Contents);
}

void KXmlGui_UnitTest::testVersionHandlerNewVersionUserChanges()
{
    // This emulates the case where the application has been upgraded
    // after the user has changed shortcuts and toolbars

    QMap<QString, int> fileToVersionMap; // makes QCOMPARE failures more readable than just temp filenames

    QFile fileV2(KStandardDirs::locateLocal("appdata", "testui.rc"));
    QVERIFY(fileV2.open(QIODevice::WriteOnly));
    createXmlFile(fileV2, 2, AddActionProperties | AddModifiedToolBars);
    fileToVersionMap.insert(fileV2.fileName(), 2);

    KTemporaryFile fileV5;
    QVERIFY(fileV5.open());
    createXmlFile(fileV5, 5, AddToolBars | AddModifiedMenus);
    fileToVersionMap.insert(fileV5.fileName(), 5);

    // The highest version is neither the first nor last one in the list,
    // to make sure the code really selects the highest version, not just by chance :)
    // (This is why we add the v1 version at the end of the list)
    KTemporaryFile fileV1;
    QVERIFY(fileV1.open());
    createXmlFile(fileV1, 1, AddToolBars);
    fileToVersionMap.insert(fileV1.fileName(), 1);


    QStringList files;
    files << fileV2.fileName() << fileV5.fileName() << fileV1.fileName();

    fileV2.close();
    fileV5.close();
    fileV1.close();

    KXmlGuiVersionHandler versionHandler(files);
    // We selected the local file, so in our map it has version 2.
    // But of course by now it says "version=5" in it :)
    QCOMPARE(fileToVersionMap.value(versionHandler.finalFile()), 2);
    const QString finalDoc = versionHandler.finalDocument();
    //kDebug() << finalDoc;
    QVERIFY(finalDoc.startsWith("<?xml"));
    QVERIFY(finalDoc.contains("version=\"5\""));
    // Check that the shortcuts defined by the user were kept
    QVERIFY(finalDoc.contains("<ActionProperties>"));
    QVERIFY(finalDoc.contains("sidebartng"));
    // Check that the menus modified by the app are still there
    QVERIFY(finalDoc.contains("<Action name=\"file_open\""));
    // Check that the toolbars modified by the user were kept
    QVERIFY(finalDoc.contains("<Action name=\"home\""));
}
