/* This file is part of the KDE libraries

    Copyright 2007-2009 David Faure <faure@kde.org>

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
#include <QShowEvent>
#include <kedittoolbar.h>
#include <kaction.h>
#include <kpushbutton.h>
#include <kmenubar.h>
#include <kxmlguibuilder.h>
#include <kxmlguiclient.h>
#include "testxmlguiwindow.h"
#include "testguiclient.h"
#include "kxmlgui_unittest.moc"
#include <kxmlguiversionhandler_p.h>
#include <kxmlguiversionhandler.cpp> // it's not exported, so we need to include the code here

QTEST_KDEMAIN(KXmlGui_UnitTest, GUI)

enum Flags {
    NoFlags = 0,
    AddToolBars = 1,
    AddModifiedToolBars = 2,
    AddActionProperties = 4,
    AddModifiedMenus = 8
    // next item is 16
};

static void createXmlFile(QFile& file, int version, int flags, const QByteArray& toplevelTag = "gui")
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<" + toplevelTag + " version=\"" + QByteArray::number(version) + "\" name=\"foo\" >\n"
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
            "<ToolBar name=\"mainToolBar\">\n"
            "  <text>Main Toolbar</text>\n"
            "  <Action name=\"print\" />\n"
            "</ToolBar>\n"
            "<ToolBar name=\"bookmarkToolBar\">\n"
            "  <text>Bookmark Toolbar</text>\n"
            "</ToolBar>\n"
            );
    }
    if (flags & AddModifiedToolBars) {
        file.write(
            "<ToolBar name=\"mainToolBar\">\n"
            "  <text>Main Toolbar</text>\n"
            "  <Action name=\"home\" />\n"
            "</ToolBar>\n"
            "<ToolBar name=\"bookmarkToolBar\">\n"
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
    file.write("</" + toplevelTag + ">\n");
}


void KXmlGui_UnitTest::testFindVersionNumber_data()
{
    QTest::addColumn<QString>("xml");
    QTest::addColumn<QString>("version");

    QTest::newRow("simple test") <<
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"3\" name=\"foo\"/>\n" << "3";
    QTest::newRow("two digits") <<
        "<?xml version = '1.0'?>\n"
        "<kpartgui version=\"42\" name=\"foo\"/>\n" << "42";
    QTest::newRow("with spaces") << // as found in dirfilterplugin.rc for instance
        "<?xml version = '1.0'?>\n"
        "<gui version = \"1\" name=\"foo\"/>\n" << "1";
    QTest::newRow("with a dot") << // as was found in autorefresh.rc
        "<?xml version = '1.0'?>\n"
        "<gui version = \"0.2\" name=\"foo\"/>\n" << QString() /*error*/;
    QTest::newRow("with a comment") << // as found in kmail.rc
        "<!DOCTYPE kpartgui>\n"
        "<!-- This file should be synchronized with kmail_part.rc to provide\n"
        "the same menu entries at the same place in KMail and Kontact  -->\n"
        "<kpartgui version=\"452\" name=\"kmmainwin\">\n" << "452";
}

void KXmlGui_UnitTest::testFindVersionNumber()
{
    QFETCH(QString, xml);
    QFETCH(QString, version);
    QCOMPARE(KXmlGuiVersionHandler::findVersionNumber(xml), version);
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

    // local file
    QFile fileV2(KStandardDirs::locateLocal("appdata", "testui.rc"));
    QVERIFY(fileV2.open(QIODevice::WriteOnly));
    createXmlFile(fileV2, 2, AddActionProperties | AddModifiedToolBars);
    fileToVersionMap.insert(fileV2.fileName(), 2);

    // more-global file
    KTemporaryFile fileV5;
    QVERIFY(fileV5.open());
    createXmlFile(fileV5, 5, AddToolBars | AddModifiedMenus, "kpartgui");
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
    // We end up with the local file, so in our map it has version 2.
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

static QStringList collectMenuNames(KXMLGUIFactory& factory)
{
    QList<QWidget*> containers = factory.containers("Menu");
    QStringList containerNames;
    Q_FOREACH(QWidget* w, containers) {
        containerNames << w->objectName();
    }
    return containerNames;
}

#if 0
static void debugActions(const QList<QAction*>& actions)
{
    Q_FOREACH(QAction* action, actions)
        kDebug() << (action->isSeparator() ? QString("separator") : action->objectName());
}
#endif

static void checkActions(const QList<QAction*>& actions, const QStringList& expectedActions)
{
    for (int i = 0; i < expectedActions.count(); ++i) {
        QAction* action = actions[i];
        if (action->isSeparator())
            QCOMPARE(QString("separator"), expectedActions[i]);
        else
            QCOMPARE(action->objectName(), expectedActions[i]);
    }
    QCOMPARE(actions.count(), expectedActions.count());
}

void KXmlGui_UnitTest::testPartMerging()
{
    const QByteArray hostXml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"go\"><text>&amp;Go</text>\n"
        "  <!-- go_up, go_back, go_forward, go_home: coming from ui_standards.rc -->\n"
        "  <Merge/>\n"
        "  <Action name=\"host_after_merge\"/>\n"
        "  <Action name=\"host_after_merge_2\"/>\n"
        "  <Separator/>\n"
        "  <DefineGroup name=\"new_merge\"/>\n"
        "  <Action name=\"last_from_host\"/>\n"
        " </Menu>\n"
        " <Menu name=\"file\"><text>&amp;File</text>\n"
        "  <DefineGroup name=\"placed_merge\" append=\"new_merge\"/>\n"
        " </Menu>\n"
        "</MenuBar>\n"
        "</gui>\n";


    TestGuiClient hostClient;
    hostClient.createActions(QStringList() << "go_up" << "go_back" << "go_forward" << "go_home"
                             << "host_after_merge" << "host_after_merge_2" << "last_from_host"
                             << "file_new" << "file_open" << "file_quit");
    hostClient.createGUI(hostXml, true /*ui_standards.rc*/);
    QMainWindow mainWindow;
    KXMLGUIBuilder builder(&mainWindow);
    KXMLGUIFactory factory(&builder);
    factory.addClient(&hostClient);

    QWidget* goMenuW = factory.container("go", &hostClient);
    QVERIFY(goMenuW);
    QMenu* goMenu = qobject_cast<QMenu *>(goMenuW);
    QVERIFY(goMenu);
    QMenu* fileMenu = qobject_cast<QMenu *>(factory.container("file", &hostClient));

    //debugActions(goMenu->actions());
    checkActions(goMenu->actions(), QStringList()
                 << "go_up"
                 << "go_back"
                 << "go_forward"
                 << "go_home"
                 << "separator"
                 << "host_after_merge"
                 << "host_after_merge_2"
                 << "separator"
                 << "last_from_host");
    checkActions(fileMenu->actions(), QStringList()
                 << "file_new"
                 << "file_open"
                 << "separator"
                 << "file_quit");

    kDebug() << "Now merging the part";

    const QByteArray partXml =
        "<!DOCTYPE kpartgui SYSTEM \"kpartgui.dtd\">\n"
        "<kpartgui version=\"1\" name=\"part\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"go\"><text>&amp;Go</text>\n"
        "  <Action name=\"go_previous\"/>\n"
        "  <Action name=\"go_next\"/>\n"
        "  <Separator/>\n"
        "  <Action name=\"first_page\"/>\n"
        "  <Action name=\"last_page\"/>\n"
        "  <Separator/>\n"
        "  <Action name=\"action_in_merge_group\" group=\"new_merge\"/>\n"
        "  <Action name=\"undefined_group\" group=\"no_such_merge\"/>\n"
        "  <Action name=\"last_from_part\"/>\n"
        " </Menu>\n"
        " <Menu name=\"file\"><text>&amp;File</text>\n"
        "  <Action group=\"placed_merge\" name=\"action_in_placed_merge\"/>\n"
        "  <Action name=\"other_file_action\"/>\n"
        " </Menu>\n"
        "</MenuBar>\n"
        "</kpartgui>\n";

    TestGuiClient partClient(partXml);
    partClient.createActions(QStringList() << "go_previous" << "go_next" << "first_page" <<
            "last_page" << "last_from_part" << "action_in_merge_group" << "undefined_group" <<
            "action_in_placed_merge" << "other_file_action" );
    factory.addClient(&partClient);

    //debugActions(goMenu->actions());
    checkActions(goMenu->actions(), QStringList()
                 << "go_up"
                 << "go_back"
                 << "go_forward"
                 << "go_home"
                 << "separator"
                 // Contents of the <Merge>:
                 << "go_previous"
                 << "go_next"
                 << "separator"
                 << "first_page"
                 << "last_page"
                 << "separator"
                 << "undefined_group"
                 << "last_from_part"
                 // End of <Merge>
                 << "host_after_merge"
                 << "host_after_merge_2"
                 << "separator"
                 // Contents of <DefineGroup>
                 << "action_in_merge_group"
                 // End of <DefineGroup>
                 << "last_from_host"
        );
        checkActions(fileMenu->actions(), QStringList()
                 << "file_new"
                 << "action_in_placed_merge"
                 << "file_open"
                 << "separator"
                 << "file_quit"
                 << "other_file_action");
}

void KXmlGui_UnitTest::testUiStandardsMerging_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QStringList>("actions");
    QTest::addColumn<QStringList>("expectedMenus");

    const QByteArray xmlBegin =
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n";
    const QByteArray xmlEnd =
        "</MenuBar>\n"
        "</gui>";

    // Merging an empty menu (or a menu with only non-existing actions) would make
    // the empty menu appear at the end after all other menus (fixed for KDE-4.2)
    QTest::newRow("empty file menu, implicit settings menu")
        << xmlBegin + "<Menu name=\"file\"/>\n" + xmlEnd
        << (QStringList() << "options_configure_toolbars")
        << (QStringList() << "settings");
    QTest::newRow("file menu with non existing action, implicit settings menu")
        << xmlBegin + "<Menu name=\"file\"><Action name=\"foo\"/></Menu>\n" + xmlEnd
        << (QStringList() << "options_configure_toolbars")
        << (QStringList() << "settings");
    QTest::newRow("file menu with existing action, implicit settings menu")
        << xmlBegin + "<Menu name=\"file\"><Action name=\"open\"/></Menu>\n" + xmlEnd
        << (QStringList() << "open" << "options_configure_toolbars")
        << (QStringList() << "file" << "settings");
    QTest::newRow("implicit file and settings menu")
        << xmlBegin + xmlEnd
        << (QStringList() << "file_open" << "options_configure_toolbars")
        << (QStringList() << "file" << "settings"); // we could check that file_open is in the mainToolBar, too

    // Check that unknown non-empty menus are added at the "MergeLocal" position (before settings).
    QTest::newRow("foo menu added at the end")
        << xmlBegin + "<Menu name=\"foo\"><Action name=\"foo_action\"/></Menu>\n" + xmlEnd
        << (QStringList() << "file_open" << "options_configure_toolbars" << "foo_action")
        << (QStringList() << "file" << "foo" << "settings");

    QTest::newRow("Bille's testcase: menu patch + menu edit")
        << xmlBegin + "<Menu name=\"patch\"><Action name=\"patch_generate\"/></Menu>\n"
        + "<Menu name=\"edit\"><Action name=\"edit_foo\"/></Menu>\n"
        + xmlEnd
        << (QStringList() << "file_open" << "patch_generate" << "edit_foo")
        << (QStringList() << "file" << "edit" << "patch");
    QTest::newRow("Bille's testcase: menu patch + menu edit, lowercase tag")
        << xmlBegin + "<Menu name=\"patch\"><Action name=\"patch_generate\"/></Menu>\n"
        + "<menu name=\"edit\"><Action name=\"edit_foo\"/></menu>\n"
        + xmlEnd
        << (QStringList() << "file_open" << "patch_generate" << "edit_foo")
        << (QStringList() << "file" << "edit" << "patch");

    // Check that <Menu append="..."> allows to insert menus at specific positions
    QTest::newRow("Menu append")
        << xmlBegin + "<Menu name=\"foo\" append=\"settings_merge\"><Action name=\"foo_action\"/></Menu>\n" + xmlEnd
        << (QStringList() << "file_open" << "options_configure_toolbars" << "foo_action" << "help_contents")
        << (QStringList() << "file" << "settings" << "foo" << "help");
    QTest::newRow("Custom first menu")
        << xmlBegin + "<Menu name=\"foo\" append=\"first_menu\"><Action name=\"foo_action\"/></Menu>\n" + xmlEnd
        << (QStringList() << "edit_undo" << "foo_action" << "help_contents")
        << (QStringList() << "foo" << "edit" << "help");

    // Tests for noMerge="1"
    QTest::newRow("noMerge empty file menu, implicit settings menu")
        << xmlBegin + "<Menu name=\"file\" noMerge=\"1\"/>\n" + xmlEnd
        << (QStringList() << "file_open" << "options_configure_toolbars")
        << (QStringList() << "file" << "settings"); // we keep empty menus, see #186382
    QTest::newRow("noMerge empty file menu, file_open moved elsewhere")
        << xmlBegin + "<Menu name=\"file\" noMerge=\"1\"/>\n<Menu name=\"foo\"><Action name=\"file_open\"/></Menu>" + xmlEnd
        << (QStringList() << "file_open")
        << (QStringList() << "file" << "foo");
    QTest::newRow("noMerge file menu with open before new")
        << xmlBegin + "<Menu name=\"file\" noMerge=\"1\"><Action name=\"file_open\"/><Action name=\"file_new\"/></Menu>" + xmlEnd
        << (QStringList() << "file_open" << "file_new")
        << (QStringList() << "file"); // TODO check the order of the actions in the menu? how?

    // Tests for deleted="true"
    QTest::newRow("deleted file menu, implicit settings menu")
        << xmlBegin + "<Menu name=\"file\" deleted=\"true\"/>\n" + xmlEnd
        << (QStringList() << "file_open" << "options_configure_toolbars")
        << (QStringList() << "settings");
    QTest::newRow("deleted file menu, file_open moved elsewhere")
        << xmlBegin + "<Menu name=\"file\" deleted=\"true\"/>\n<Menu name=\"foo\"><Action name=\"file_open\"/></Menu>" + xmlEnd
        << (QStringList() << "file_open")
        << (QStringList() << "foo");
    QTest::newRow("deleted file menu with actions (contradiction)")
        << xmlBegin + "<Menu name=\"file\" deleted=\"true\"><Action name=\"file_open\"/><Action name=\"file_new\"/></Menu>" + xmlEnd
        << (QStringList() << "file_open" << "file_new")
        << (QStringList());

}

void KXmlGui_UnitTest::testUiStandardsMerging()
{
    QFETCH(QByteArray, xml);
    QFETCH(QStringList, actions);
    QFETCH(QStringList, expectedMenus);

    TestGuiClient client;
    client.createActions(actions);
    client.createGUI(xml, true /*ui_standards.rc*/);

    const QDomDocument domDocument = client.domDocument();
    const QDomElement docElem = domDocument.documentElement();
    QCOMPARE(docElem.attribute("name"), QString("foo")); // not standard_containers from ui_standards.rc
    QCOMPARE(docElem.attribute("version"), QString("1")); // not standard_containers from ui_standards.rc

    QMainWindow mainWindow;
    KXMLGUIBuilder builder(&mainWindow);
    KXMLGUIFactory factory(&builder);
    factory.addClient(&client);

    const QStringList containerNames = collectMenuNames(factory);
    //kDebug() << containerNames;
    QCOMPARE(containerNames, expectedMenus);
}

void KXmlGui_UnitTest::testActionListAndSeparator()
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"groups\"><text>Add to Group</text>\n"
        "  <ActionList name=\"view_groups_list\"/>\n"
        "  <Separator />"
        "   <Action name=\"view_add_to_new_group\" />\n"
        " </Menu>\n"
        "</MenuBar>\n"
        "</gui>";

    TestGuiClient client(xml);
    client.createActions(QStringList() << "view_add_to_new_group" << "action1");
    QMainWindow mainWindow;
    KXMLGUIBuilder builder(&mainWindow);
    KXMLGUIFactory factory(&builder);
    factory.addClient(&client);

    QWidget* menuW = factory.container("groups", &client);
    QVERIFY(menuW);
    QMenu* menu = qobject_cast<QMenu *>(menuW);
    QVERIFY(menu);

    //debugActions(menu->actions());
    checkActions(menu->actions(), QStringList()
                 << "separator" // that's ok, QMenuPrivate::filterActions won't show it
                 << "view_add_to_new_group");

    kDebug() << "Now plugging the actionlist";

    QList<QAction*> actionList;
    actionList << client.actionCollection()->action("action1");
    client.plugActionList("view_groups_list", actionList);

    //debugActions(menu->actions());
    checkActions(menu->actions(), QStringList()
                 << "action1"
                 << "separator"
                 << "view_add_to_new_group");

    kDebug() << "Now remove+add gui client";

    // While I'm here, what happens with the action list if I remove+add the guiclient,
    // like KXmlGuiWindow::newToolBarConfig does?
    factory.removeClient(&client);
    factory.addClient(&client);
    // We need to get the container widget again, it was re-created.
    menuW = factory.container("groups", &client);
    QVERIFY(menuW);
    menu = qobject_cast<QMenu *>(menuW);
    //debugActions(menu->actions());
    checkActions(menu->actions(), QStringList()
                 << "separator"   // yep, it removed the actionlist thing...
                 << "view_add_to_new_group");
    kDebug() << "Now plugging the actionlist again";
    client.plugActionList("view_groups_list", actionList);
    checkActions(menu->actions(), QStringList()
                 << "action1"
                 << "separator"
                 << "view_add_to_new_group");
}

void KXmlGui_UnitTest::testHiddenToolBar()
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        "</MenuBar>\n"
        "<ToolBar hidden=\"true\" name=\"mainToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"visibleToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar hidden=\"true\" name=\"hiddenToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "</gui>\n";
    KConfigGroup cg(KGlobal::config(), "testHiddenToolBar");
    TestXmlGuiWindow mainWindow(xml);
    mainWindow.setAutoSaveSettings(cg);
    mainWindow.createActions(QStringList() << "go_up");
    mainWindow.createGUI();

    KToolBar* mainToolBar = mainWindow.toolBarByName("mainToolBar");
    QVERIFY(mainToolBar->isHidden());

    KXMLGUIFactory* factory = mainWindow.guiFactory();
    QVERIFY(!factory->container("visibleToolBar", &mainWindow)->isHidden());
    KToolBar* hiddenToolBar = qobject_cast<KToolBar *>(factory->container("hiddenToolBar", &mainWindow));
    kDebug() << hiddenToolBar;
    QVERIFY(hiddenToolBar->isHidden());

    // Now open KEditToolBar (#105525)
    KEditToolBar editToolBar(factory);
    // KEditToolBar loads the stuff in showEvent...
    QShowEvent ev; qApp->sendEvent(&editToolBar, &ev);
    editToolBar.button(KDialog::Apply)->setEnabled(true);
    editToolBar.button(KDialog::Apply)->click();
    QVERIFY(qobject_cast<KToolBar *>(factory->container("hiddenToolBar", &mainWindow))->isHidden());

    mainWindow.close();
}

// taken from KMainWindow_UnitTest::testAutoSaveSettings()
void KXmlGui_UnitTest::testAutoSaveSettings()
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        "</MenuBar>\n"
        "<ToolBar name=\"mainToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"secondToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "</gui>\n";
    {
        // do not interfere with the "toolbarVisibility" unit test
        KConfigGroup cg(KGlobal::config(), "testAutoSaveSettings");
        TestXmlGuiWindow mw(xml);
        mw.setAutoSaveSettings(cg);

        // Test resizing first (like show() does).
        mw.reallyResize(400, 400);

        mw.createActions(QStringList() << "go_up");
        mw.createGUI();

        // Resize again, should be saved
        mw.reallyResize(800, 600);

        KToolBar* mainToolBar = mw.toolBarByName("mainToolBar");
        QCOMPARE(mw.toolBarArea(mainToolBar), Qt::TopToolBarArea);
        KToolBar* secondToolBar = mw.toolBarByName("secondToolBar");
        QCOMPARE(mw.toolBarArea(secondToolBar), Qt::TopToolBarArea); // REFERENCE #1 (see below)

        // Move second toolbar to bottom
        const QPoint oldPos = secondToolBar->pos();
        mw.addToolBar(Qt::BottomToolBarArea, secondToolBar);
        const QPoint newPos = secondToolBar->pos();
        QCOMPARE(mw.toolBarArea(secondToolBar), Qt::BottomToolBarArea);
        // Calling to addToolBar is not enough to trigger the event filter for move events
        // in KMainWindow, because there is no layouting happening in hidden mainwindows.
        QMoveEvent moveEvent(newPos, oldPos);
        QApplication::sendEvent(secondToolBar, &moveEvent);

        mw.close();
    }

    {
        KConfigGroup cg(KGlobal::config(), "testAutoSaveSettings");
        TestXmlGuiWindow mw2(xml);
        mw2.setAutoSaveSettings(cg);
        // Check window size was restored
        QCOMPARE(mw2.size(), QSize(800, 600));

        mw2.createActions(QStringList() << "go_up");
        mw2.createGUI();

        // Force window layout to happen
        mw2.reallyResize(800, 600);

        // Check toolbar positions were restored
        KToolBar* mainToolBar = mw2.toolBarByName("mainToolBar");
        QCOMPARE(mw2.toolBarArea(mainToolBar), Qt::TopToolBarArea);
        KToolBar* secondToolBar = mw2.toolBarByName("secondToolBar");
        QCOMPARE(mw2.toolBarArea(secondToolBar), Qt::BottomToolBarArea);
        mw2.applyMainWindowSettings(mw2.autoSaveConfigGroup());
        QCOMPARE(mw2.toolBarArea(secondToolBar), Qt::BottomToolBarArea);
    }
}

void KXmlGui_UnitTest::testDeletedContainers()
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        "  <Menu deleted=\"true\" name=\"game\"/>\n"
        "</MenuBar>\n"
        "<ToolBar deleted=\"true\" name=\"mainToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"visibleToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar deleted=\"true\" name=\"deletedToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "</gui>\n";
    KConfigGroup cg(KGlobal::config(), "testDeletedToolBar");
    TestXmlGuiWindow mainWindow(xml);
    mainWindow.setAutoSaveSettings(cg);
    mainWindow.createActions(QStringList() << "go_up" << "file_new" << "game_new");
    mainWindow.createGUI();
    KXMLGUIFactory* factory = mainWindow.guiFactory();

    //qDebug() << "containers:" << factory->containers("ToolBar");
    QVERIFY(!factory->container("mainToolBar", &mainWindow));
    QVERIFY(!factory->container("visibleToolBar", &mainWindow)->isHidden());
    QVERIFY(!factory->container("deletedToolBar", &mainWindow));
    QVERIFY(factory->container("file", &mainWindow)); // File menu was created
    QVERIFY(!factory->container("game", &mainWindow)); // Game menu was not created

    // Now open KEditToolBar, just to check it doesn't crash.
    KEditToolBar editToolBar(factory);
    // KEditToolBar loads the stuff in showEvent...
    QShowEvent ev; qApp->sendEvent(&editToolBar, &ev);
    editToolBar.button(KDialog::Apply)->setEnabled(true);
    editToolBar.button(KDialog::Apply)->click();
    QVERIFY(!factory->container("mainToolBar", &mainWindow));
    QVERIFY(!factory->container("visibleToolBar", &mainWindow)->isHidden());
    QVERIFY(!factory->container("deletedToolBar", &mainWindow));
    QVERIFY(factory->container("file", &mainWindow));
    QVERIFY(!factory->container("game", &mainWindow));

    mainWindow.close();
}

void KXmlGui_UnitTest::testTopLevelSeparator() {
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"before_separator\"><text>Before Separator</text></Menu>\n"
        " <Separator />\n"
        " <Menu name=\"after_separator\"><text>After Separator</text></Menu>\n"
        "</MenuBar>\n"
        "</gui>";

    TestXmlGuiWindow mainWindow(xml);
    mainWindow.setAutoSaveSettings(false);
    mainWindow.createGUI();

    checkActions(mainWindow.menuBar()->actions(), QStringList()
                 << "before_separator"
                 << "separator"
                 << "after_separator"
                 << "separator"
                 << "help");
}

// Check that the objectName() of the menus is set from the name in the XML file
void KXmlGui_UnitTest::testMenuNames()
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"filemenu\"><text>File Menu</text></Menu>\n"
        "</MenuBar>\n"
        "</gui>";

    TestXmlGuiWindow mainWindow(xml);
    mainWindow.setAutoSaveSettings(false);
    mainWindow.createGUI();

    checkActions(mainWindow.menuBar()->actions(), QStringList()
                 << "filemenu"
                 << "separator"
                 << "help" );
}

// Test what happens when the application's rc file isn't found
// We want a warning to be printed, but we don't want to see all menus from ui_standards.rc
void KXmlGui_UnitTest::testMenusNoXmlFile()
{
    TestXmlGuiWindow mainWindow;
    mainWindow.setAutoSaveSettings(false);
    mainWindow.createGUIBad();

    checkActions(mainWindow.menuBar()->actions(), QStringList()
                 << "help" );
}

void KXmlGui_UnitTest::testXMLFileReplacement() {
    // to differentiate "original" and replacement xml file, one is created with "modified" toolbars
    KTemporaryFile fileOrig;
    QVERIFY(fileOrig.open());
    createXmlFile(fileOrig, 2, AddToolBars);
    const QString filenameOrig = fileOrig.fileName();
    fileOrig.close();

    KTemporaryFile fileReplace;
    QVERIFY(fileReplace.open());
    createXmlFile(fileReplace, 2, AddModifiedToolBars);
    const QString filenameReplace = fileReplace.fileName();
    fileReplace.close();

    // finally, our local xml file has <ActionProperties/>
    QFile fileLocal(KStandardDirs::locateLocal("appdata", "testui.rc"));
    QVERIFY(fileLocal.open(QIODevice::WriteOnly));
    createXmlFile(fileLocal, 1, AddActionProperties);
    const QString filenameLocal = fileLocal.fileName();
    fileLocal.close();

    TestGuiClient client;
    // first make sure that the "original" file is loaded, correctly
    client.setXMLFilePublic(filenameOrig);
    QString xml = client.domDocument().toString();
    //qDebug() << xml;
    QVERIFY(xml.contains("<Action name=\"print\""));
    QVERIFY(!xml.contains("<Action name=\"home\""));
    QVERIFY(!xml.contains("<ActionProperties>"));

    // now test the replacement (+ local file)
    client.replaceXMLFile(filenameReplace, filenameLocal);
    xml = client.domDocument().toString();
    QVERIFY(!xml.contains("<Action name=\"print\""));
    QVERIFY(xml.contains("<Action name=\"home\""));
    QVERIFY(xml.contains("<ActionProperties>"));

    // re-check after a reload
    client.reloadXML();
    QString reloadedXml = client.domDocument().toString();
    QVERIFY(!reloadedXml.contains("<Action name=\"print\""));
    QVERIFY(reloadedXml.contains("<Action name=\"home\""));
    QVERIFY(reloadedXml.contains("<ActionProperties>"));

    // Check what happens when the local file doesn't exist
    TestGuiClient client2;
    QFile::remove(filenameLocal);
    client2.replaceXMLFile(filenameReplace, filenameLocal);
    xml = client2.domDocument().toString();
    //qDebug() << xml;
    QVERIFY(!xml.contains("<Action name=\"print\""));
    QVERIFY(xml.contains("<Action name=\"home\"")); // modified toolbars
    QVERIFY(!xml.contains("<ActionProperties>")); // but no local xml file
}

void KXmlGui_UnitTest::testClientDestruction() { // #170806
    const QByteArray xml = 
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"filemenu\"><text>File Menu</text></Menu>\n"
        "</MenuBar>\n"
        "</gui>";

    TestXmlGuiWindow mainWindow(xml);
    TestGuiClient* client = new TestGuiClient(xml);
    mainWindow.insertChildClient(client);
    mainWindow.setAutoSaveSettings(false);
    mainWindow.createGUI();

    QVERIFY(mainWindow.factory()->clients().contains(client));
    delete client;
    QVERIFY(!mainWindow.factory()->clients().contains(client));
}

