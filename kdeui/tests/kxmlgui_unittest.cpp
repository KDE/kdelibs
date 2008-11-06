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
#include <QShowEvent>
#include <kedittoolbar.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kpushbutton.h>
#include <QMenu>
#include <kxmlguibuilder.h>
#include <kxmlguiclient.h>
#include <ktoolbar.h>
#include <kxmlguiwindow.h>
#include <ktemporaryfile.h>
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
        "<gui version=\"42\" name=\"foo\"/>\n" << "42";
    QTest::newRow("with spaces") << // as found in dirfilterplugin.rc for instance
        "<?xml version = '1.0'?>\n"
        "<gui version = \"1\" name=\"foo\"/>\n" << "1";
    QTest::newRow("with a dot") << // as was found in autorefresh.rc
        "<?xml version = '1.0'?>\n"
        "<gui version = \"0.2\" name=\"foo\"/>\n" << QString() /*error*/;
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

// because setDOMDocument and setXML are protected
class TestGuiClient : public KXMLGUIClient
{
public:
    TestGuiClient(const QByteArray& xml, bool withUiStandards = false)
        : KXMLGUIClient()
    {
        if (withUiStandards) {
            QString uis = KStandardDirs::locate("config", "ui/ui_standards.rc", componentData());
            QVERIFY(!uis.isEmpty());
            setXMLFile(uis);
        }

        setXML(QString::fromLatin1(xml), true);
    }
};

static void createActions(KActionCollection* collection, const QStringList& actionNames)
{
    Q_FOREACH(const QString& actionName, actionNames) {
        collection->addAction(actionName)->setText("Action");
    }
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
}

void KXmlGui_UnitTest::testMergingSeparators()
{
    const QByteArray hostXml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"go\"><text>&amp;Go</text>\n"
//        "  <!-- TODO: go_up, go_back, go_forward, go_home: coming from ui_standards.rc -->\n"
        "  <Action name=\"go_up\"/>\n"
        "  <Action name=\"go_back\"/>\n"
        "  <Action name=\"go_forward\"/>\n"
        "  <Action name=\"go_home\"/>\n"
        "  <Merge/>\n"
        "  <Action name=\"go_history\"/>\n"
        "  <Action name=\"go_most_often\"/>\n"
        "  <Separator/>\n"
        "  <Action name=\"history\"/>\n"
        "  <Action name=\"closedtabs\"/>\n"
        " </Menu>\n"
        "</MenuBar>\n"
        "</gui>\n";


    TestGuiClient hostClient(hostXml, true /*ui_standards.rc*/);
    createActions(hostClient.actionCollection(),
                  QStringList() << "go_up" << "go_back" << "go_forward" << "go_home"
                  << "go_history" << "go_most_often");
    QMainWindow mainWindow;
    KXMLGUIBuilder builder(&mainWindow);
    KXMLGUIFactory factory(&builder);
    factory.addClient(&hostClient);

    QWidget* goMenuW = factory.container("go", &hostClient);
    QVERIFY(goMenuW);
    QMenu* goMenu = qobject_cast<QMenu *>(goMenuW);
    QVERIFY(goMenu);

    //debugActions(goMenu->actions());
    checkActions(goMenu->actions(), QStringList()
                 << "go_up"
                 << "go_back"
                 << "go_forward"
                 << "go_home"
                 << "go_history"
                 << "go_most_often"
                 << "separator");

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
        "  <Action name=\"go_document_back\"/>\n"
        "  <Action name=\"go_document_forward\" />\n"
        "  <Separator/>\n"
        "  <Action name=\"go_goto_page\"/>\n"
        " </Menu>\n"
        "</MenuBar>\n"
        "</kpartgui>\n";

    TestGuiClient partClient(partXml);
    createActions(partClient.actionCollection(),
                  QStringList() << "go_previous" << "go_next" << "first_page" << "last_page");
    factory.addClient(&partClient);

    //debugActions(goMenu->actions());
    checkActions(goMenu->actions(), QStringList()
                 << "go_up"
                 << "go_back"
                 << "go_forward"
                 << "go_home"
                 << "go_previous"
                 << "go_next"
                 << "separator"
                 << "first_page");
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
    createActions(client.actionCollection(),
                  QStringList() << "view_add_to_new_group" << "action1");
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

class TestXmlGuiWindow : public KXmlGuiWindow
{
public:
    TestXmlGuiWindow(const QByteArray& xml) : KXmlGuiWindow() {
        QVERIFY(m_userFile.open());
        m_userFile.write(xml);
        m_fileName = m_userFile.fileName(); // remember filename
        Q_ASSERT(!m_fileName.isEmpty());
        m_userFile.close(); // write to disk
    }
    void createGUI() {
        KXmlGuiWindow::createGUI(m_fileName);
    }
private:
    KTemporaryFile m_userFile;
    QString m_fileName;
};

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
    TestXmlGuiWindow mainWindow(xml);
    mainWindow.setAutoSaveSettings();
    createActions(mainWindow.actionCollection(), QStringList() << "go_up");
    mainWindow.createGUI();
    KXMLGUIFactory* factory = mainWindow.guiFactory();

    //qDebug() << "containers:" << factory->containers("ToolBar");
    QWidget* mainToolBarW = factory->container("mainToolBar", &mainWindow);
    QVERIFY(mainToolBarW);
    KToolBar* mainToolBar = qobject_cast<KToolBar *>(mainToolBarW);
    QVERIFY(mainToolBar);
    QVERIFY(mainToolBar->isHidden());

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
        "<ToolBar hidden=\"true\" name=\"mainToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "</gui>\n";
    TestXmlGuiWindow mw(xml);
    mw.setAutoSaveSettings();

    // Test resizing first (like show() does).
    mw.resize(400, 400);
    // Send the pending resize event (resize() only sets Qt::WA_PendingResizeEvent)
    QResizeEvent e(mw.size(), QSize());
    QApplication::sendEvent(&mw, &e);

    createActions(mw.actionCollection(), QStringList() << "go_up");
    mw.createGUI();

    // Resize again, should be saved
    mw.resize(800, 600);
    // Send the pending resize event (resize() only sets Qt::WA_PendingResizeEvent)
    QResizeEvent e2(mw.size(), QSize());
    QApplication::sendEvent(&mw, &e2);

    mw.close();

    TestXmlGuiWindow mw2(xml);
    mw2.setAutoSaveSettings();
    QCOMPARE(mw2.size(), QSize(800, 600));
}
