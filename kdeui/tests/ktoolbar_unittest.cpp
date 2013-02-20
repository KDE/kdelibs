/* This file is part of the KDE libraries
    Copyright (c) 2009 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kglobalsettings.h>
#include <QtTest/QtTest>
#include <QtTestWidgets>
#include <QtTest/QtTest>
#include <ktoolbar.h>
#include <kmainwindow.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <QDomElement>
#include <QToolButton>
#include "testxmlguiwindow.h"
#include "testguiclient.h"

// We use the data types below in a QVariant, so Q_DECLARE_METATYPE is needed for them.
Q_DECLARE_METATYPE(Qt::MouseButton)
Q_DECLARE_METATYPE(Qt::MouseButtons)
Q_DECLARE_METATYPE(Qt::KeyboardModifiers)

class tst_KToolBar : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void ktoolbar();

    void testIconSizeNoXmlGui_data();
    void testIconSizeNoXmlGui();
    void testIconSizeXmlGui_data();
    void testIconSizeXmlGui();
    void testToolButtonStyleNoXmlGui_data();
    void testToolButtonStyleNoXmlGui();
    void testToolButtonStyleXmlGui_data();
    void testToolButtonStyleXmlGui();
    void testToolBarPosition();
    void testXmlGuiSwitching();
    void testClickingToolButton_data();
    void testClickingToolButton();

protected:
    bool eventFilter(QObject * watched, QEvent * event);

private:
    void changeGlobalIconSizeSetting(int, int);
    void deleteGlobalIconSizeSetting();
    void changeGlobalToolButtonStyleSetting(const QString&, const QString&);
    void deleteGlobalToolButtonStyleSetting();
    QByteArray m_xml;
    bool m_showWasCalled;
};

QTEST_MAIN(tst_KToolBar)

// This will be called before the first test function is executed.
// It is only called once.
void tst_KToolBar::initTestCase()
{
    m_xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        "</MenuBar>\n"
        "<ToolBar name=\"mainToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"otherToolBar\" position=\"bottom\" iconText=\"TextUnderIcon\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"cleanToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"hiddenToolBar\" hidden=\"true\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"secondHiddenToolBar\" hidden=\"true\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar iconSize=\"32\" name=\"bigToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar iconSize=\"32\" name=\"bigUnchangedToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "</gui>\n";

    qRegisterMetaType<Qt::MouseButtons>("Qt::MouseButtons");
    qRegisterMetaType<Qt::KeyboardModifiers>("Qt::KeyboardModifiers");
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_KToolBar::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_KToolBar::init()
{
}

// This will be called after every test function.
void tst_KToolBar::cleanup()
{
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + "tst_KToolBar");
    deleteGlobalIconSizeSetting();
    deleteGlobalToolButtonStyleSetting();
}

void tst_KToolBar::ktoolbar()
{
    KMainWindow kmw;
    // Creating a KToolBar directly
    KToolBar bar(&kmw);
    QCOMPARE(bar.mainWindow(), &kmw);
    // Asking KMainWindow for a KToolBar (more common)
    KToolBar* mainToolBar = kmw.toolBar("mainToolBar");
    QCOMPARE(mainToolBar->mainWindow(), &kmw);
}

Q_DECLARE_METATYPE(KConfigGroup)

void tst_KToolBar::testIconSizeNoXmlGui_data()
{
    QTest::addColumn<int>("iconSize");
    QTest::newRow("16") << 16;
    QTest::newRow("22") << 22;
    QTest::newRow("32") << 32;
    QTest::newRow("64") << 64;
}

void tst_KToolBar::testIconSizeNoXmlGui()
{
    QFETCH(int, iconSize);
    KConfig config("tst_KToolBar");
    KConfigGroup group(&config, "group");
    {
        KMainWindow kmw;
        KToolBar* mainToolBar = kmw.toolBar("mainToolBar");
        KToolBar* otherToolBar = kmw.toolBar("otherToolBar");

        // Default settings (applied by applyAppearanceSettings)
        QCOMPARE(mainToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::MainToolbar));
        QCOMPARE(otherToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::Toolbar));
        // check the actual values - update this if kicontheme's defaults are changed
        QCOMPARE(KIconLoader::global()->currentSize(KIconLoader::MainToolbar), 22);
        QCOMPARE(KIconLoader::global()->currentSize(KIconLoader::Toolbar), 22);

        // Changing settings for a given toolbar, as user
        mainToolBar->setIconDimensions(iconSize);
        otherToolBar->setIconDimensions(iconSize);

        // Save settings
        kmw.saveMainWindowSettings(group);
        QCOMPARE(group.groupList().count(), 2); // two subgroups (one for each toolbar)
        // was it the default value?
        if (iconSize == KIconLoader::global()->currentSize(KIconLoader::MainToolbar))
            QVERIFY(!group.group("Toolbar mainToolBar").hasKey("IconSize"));
        else
            QVERIFY(group.group("Toolbar mainToolBar").hasKey("IconSize"));
    }

    {
        // Recreate, load, compare.
        KMainWindow kmw;
        KToolBar* mainToolBar = kmw.toolBar("mainToolBar");
        KToolBar* otherToolBar = kmw.toolBar("otherToolBar");
        KToolBar* cleanToolBar = kmw.toolBar("cleanToolBar");
        QCOMPARE(mainToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::MainToolbar));
        QCOMPARE(otherToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::Toolbar));
        QCOMPARE(cleanToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::Toolbar));
        kmw.applyMainWindowSettings(group);
        QCOMPARE(mainToolBar->iconSize().width(), iconSize);
        QCOMPARE(otherToolBar->iconSize().width(), iconSize);
        QCOMPARE(cleanToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::Toolbar)); // unchanged
        const bool mainToolBarWasUsingDefaultSize = iconSize == KIconLoader::global()->currentSize(KIconLoader::MainToolbar);
        const bool otherToolBarWasUsingDefaultSize = iconSize == KIconLoader::global()->currentSize(KIconLoader::Toolbar);

        // Now emulate a change of the kde-global setting (#168480#c12)
        changeGlobalIconSizeSetting(32, 33);

        QCOMPARE(KIconLoader::global()->currentSize(KIconLoader::MainToolbar), 32);
        QCOMPARE(KIconLoader::global()->currentSize(KIconLoader::Toolbar), 33);

        if (mainToolBarWasUsingDefaultSize) {
            QCOMPARE(mainToolBar->iconSize().width(), 32);
        } else { // the user chose a specific size for the toolbar, so the new global size isn't used
            QCOMPARE(mainToolBar->iconSize().width(), iconSize);
        }
        if (otherToolBarWasUsingDefaultSize) {
            QCOMPARE(otherToolBar->iconSize().width(), 33);
        } else { // the user chose a specific size for the toolbar, so the new global size isn't used
            QCOMPARE(otherToolBar->iconSize().width(), iconSize);
        }
        QCOMPARE(cleanToolBar->iconSize().width(), 33);
    }
}

void tst_KToolBar::testIconSizeXmlGui_data()
{
    QTest::addColumn<int>("iconSize"); // set by user and saved to KConfig
    QTest::addColumn<int>("expectedSizeMainToolbar"); // ... after kde-global is changed to 25
    QTest::addColumn<int>("expectedSizeOtherToolbar"); // ... after kde-global is changed to 16
    QTest::addColumn<int>("expectedSizeCleanToolbar"); // ... after kde-global is changed to 16
    QTest::addColumn<int>("expectedSizeBigToolbar"); // ... after kde-global is changed to 16
    // When the user chose a specific size for the toolbar (!= its default size), the new kde-global size isn't applied to that toolbar.
    // So, only in case the toolbar was at iconSize already, and there was no setting in xml, we end up with kdeGlobal being used:
    const int kdeGlobalMain = 25;
    const int kdeGlobalOther = 16;
    QTest::newRow("16") << 16 << 16            << 16             << kdeGlobalOther << 16;
    QTest::newRow("22") << 22 << kdeGlobalMain << kdeGlobalOther << kdeGlobalOther << 22;
    QTest::newRow("32") << 32 << 32            << 32             << kdeGlobalOther << 32;
    QTest::newRow("64") << 64 << 64            << 64             << kdeGlobalOther << 64;
}

void tst_KToolBar::testIconSizeXmlGui()
{
    QFETCH(int, iconSize);
    QFETCH(int, expectedSizeMainToolbar);
    QFETCH(int, expectedSizeOtherToolbar);
    QFETCH(int, expectedSizeCleanToolbar);
    QFETCH(int, expectedSizeBigToolbar);
    KConfig config("tst_KToolBar");
    KConfigGroup group(&config, "group");
    {
        TestXmlGuiWindow kmw(m_xml);
        kmw.createActions(QStringList() << "go_up");
        kmw.createGUI();
        KToolBar* mainToolBar = kmw.toolBarByName("mainToolBar");
        KToolBar* otherToolBar = kmw.toolBarByName("otherToolBar");
        KToolBar* cleanToolBar = kmw.toolBarByName("cleanToolBar");
        KToolBar* bigToolBar = kmw.toolBarByName("bigToolBar");
        KToolBar* bigUnchangedToolBar = kmw.toolBarByName("bigUnchangedToolBar");

        // Default settings (applied by applyAppearanceSettings)
        QCOMPARE(mainToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::MainToolbar));
        QCOMPARE(otherToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::Toolbar));
        // check the actual values - update this if kicontheme's defaults are changed
        QCOMPARE(mainToolBar->iconSize().width(), 22);
        QCOMPARE(otherToolBar->iconSize().width(), 22);
        QCOMPARE(cleanToolBar->iconSize().width(), 22);
        QCOMPARE(bigToolBar->iconSize().width(), 32);
        QCOMPARE(bigUnchangedToolBar->iconSize().width(), 32);

        // Changing settings for a given toolbar, as user (to test the initial report in #168480)
        mainToolBar->setIconDimensions(iconSize);
        otherToolBar->setIconDimensions(iconSize);
        bigToolBar->setIconDimensions(iconSize);

        // Save settings
        kmw.saveMainWindowSettings(group);
        QVERIFY(group.groupList().count() >= 6); // one subgroup for each toolbar
        // was it the default size? (for the main toolbar, we only check that one)
        const bool usingDefaultSize = iconSize == KIconLoader::global()->currentSize(KIconLoader::MainToolbar);
        if (usingDefaultSize)
            QVERIFY(!group.group("Toolbar mainToolBar").hasKey("IconSize"));
        else
            QVERIFY(group.group("Toolbar mainToolBar").hasKey("IconSize"));

        // Now emulate a change of the kde-global setting (#168480#c12)
        changeGlobalIconSizeSetting(25, 16);

        QCOMPARE(mainToolBar->iconSize().width(), expectedSizeMainToolbar);
        QCOMPARE(otherToolBar->iconSize().width(), expectedSizeOtherToolbar);
        QCOMPARE(cleanToolBar->iconSize().width(), expectedSizeCleanToolbar);
        QCOMPARE(bigToolBar->iconSize().width(), expectedSizeBigToolbar);

        // The big unchanged toolbar should be, well, unchanged; AppXml has priority over KDE_Default.
        QCOMPARE(bigUnchangedToolBar->iconSize().width(), 32);
    }
}

void tst_KToolBar::changeGlobalIconSizeSetting(int mainToolbarIconSize, int iconSize)
{
    // We could use KConfig::Normal|KConfig::Global here, to write to kdeglobals like kcmstyle does,
    // but we don't need to. Writing to the app's config file works too.
    KConfigGroup mglobals(KSharedConfig::openConfig(), "MainToolbarIcons");
    mglobals.writeEntry("Size", mainToolbarIconSize);
    KConfigGroup globals(KSharedConfig::openConfig(), "ToolbarIcons");
    //globals.writeEntry("Size", iconSize, KConfig::Normal|KConfig::Global);
    globals.writeEntry("Size", iconSize);
    KSharedConfig::openConfig()->sync();
    QMetaObject::invokeMethod(KGlobalSettings::self(), "_k_slotNotifyChange", Q_ARG(int, KGlobalSettings::IconChanged), Q_ARG(int, 0));
}

void tst_KToolBar::deleteGlobalIconSizeSetting()
{
    KConfigGroup mglobals(KSharedConfig::openConfig(), "MainToolbarIcons");
    mglobals.deleteEntry("Size");
    KConfigGroup globals(KSharedConfig::openConfig(), "ToolbarIcons");
    globals.deleteEntry("Size");
    KSharedConfig::openConfig()->sync();
    QMetaObject::invokeMethod(KGlobalSettings::self(), "_k_slotNotifyChange", Q_ARG(int, KGlobalSettings::IconChanged), Q_ARG(int, 0));
}

Q_DECLARE_METATYPE(Qt::ToolButtonStyle)

void tst_KToolBar::testToolButtonStyleNoXmlGui_data()
{
    QTest::addColumn<Qt::ToolButtonStyle>("toolButtonStyle");
    QTest::newRow("Qt::ToolButtonIconOnly") << Qt::ToolButtonIconOnly;
    QTest::newRow("Qt::ToolButtonTextOnly") << Qt::ToolButtonTextOnly;
    QTest::newRow("Qt::ToolButtonTextBesideIcon") << Qt::ToolButtonTextBesideIcon;
    QTest::newRow("Qt::ToolButtonTextUnderIcon") << Qt::ToolButtonTextUnderIcon;
}

void tst_KToolBar::testToolButtonStyleNoXmlGui()
{
    QFETCH(Qt::ToolButtonStyle, toolButtonStyle);
    const Qt::ToolButtonStyle mainToolBarDefaultStyle = Qt::ToolButtonTextBesideIcon; // was TextUnderIcon before KDE-4.4.0
    const bool selectedDefaultForMainToolbar = toolButtonStyle == mainToolBarDefaultStyle;
    const bool selectedDefaultForOtherToolbar = toolButtonStyle == Qt::ToolButtonTextBesideIcon;

    KConfig config("tst_KToolBar");
    KConfigGroup group(&config, "group");
    {
        KMainWindow kmw;
        KToolBar* mainToolBar = kmw.toolBar("mainToolBar");
        KToolBar* otherToolBar = kmw.toolBar("otherToolBar");

        // Default settings (applied by applyAppearanceSettings)
        QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)mainToolBarDefaultStyle);
        QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)Qt::ToolButtonTextBesideIcon); // see r883541
        QCOMPARE(kmw.toolBarArea(mainToolBar), Qt::TopToolBarArea);

        // Changing settings for a given toolbar, as user
        mainToolBar->setToolButtonStyle(toolButtonStyle);
        otherToolBar->setToolButtonStyle(toolButtonStyle);

        // Save settings
        kmw.saveMainWindowSettings(group);
        QCOMPARE(group.groupList().count(), 2); // two subgroups (one for each toolbar)
        if (selectedDefaultForMainToolbar)
            QVERIFY(!group.group("Toolbar mainToolBar").hasKey("ToolButtonStyle"));
        else
            QVERIFY(group.group("Toolbar mainToolBar").hasKey("ToolButtonStyle"));
    }

    {
        // Recreate, load, compare.
        KMainWindow kmw;
        KToolBar* mainToolBar = kmw.toolBar("mainToolBar");
        KToolBar* otherToolBar = kmw.toolBar("otherToolBar");
        QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)mainToolBarDefaultStyle);
        kmw.applyMainWindowSettings(group);
        QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)toolButtonStyle);
        QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)toolButtonStyle);

        // Now change KDE-global setting
        changeGlobalToolButtonStyleSetting("IconOnly", "TextOnly");

        if (selectedDefaultForMainToolbar)
            QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)Qt::ToolButtonIconOnly);
        else
            QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)toolButtonStyle);

        if (selectedDefaultForOtherToolbar)
            QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)Qt::ToolButtonTextOnly);
        else
            QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)toolButtonStyle);
    }
}

void tst_KToolBar::testToolButtonStyleXmlGui_data()
{
    QTest::addColumn<Qt::ToolButtonStyle>("toolButtonStyle");
    // Expected style after KDE-global is changed to main=IconOnly/other=TextOnly
    QTest::addColumn<Qt::ToolButtonStyle>("expectedStyleMainToolbar");
    QTest::addColumn<Qt::ToolButtonStyle>("expectedStyleOtherToolbar"); // xml says text-under-icons, user-selected should always win
    QTest::addColumn<Qt::ToolButtonStyle>("expectedStyleCleanToolbar"); // should always follow kde-global -> always textonly.

    QTest::newRow("Qt::ToolButtonTextUnderIcon") << Qt::ToolButtonTextUnderIcon <<
        Qt::ToolButtonTextUnderIcon << Qt::ToolButtonTextUnderIcon << Qt::ToolButtonTextOnly;
    QTest::newRow("Qt::ToolButtonTextBesideIcon") << Qt::ToolButtonTextBesideIcon <<
        Qt::ToolButtonIconOnly /* was default -> using kde global */ << Qt::ToolButtonTextBesideIcon << Qt::ToolButtonTextOnly;
    QTest::newRow("Qt::ToolButtonIconOnly") << Qt::ToolButtonIconOnly <<
        Qt::ToolButtonIconOnly << Qt::ToolButtonIconOnly << Qt::ToolButtonTextOnly;
    QTest::newRow("Qt::ToolButtonTextOnly") << Qt::ToolButtonTextOnly <<
        Qt::ToolButtonTextOnly << Qt::ToolButtonTextOnly << Qt::ToolButtonTextOnly;
}

void tst_KToolBar::testToolButtonStyleXmlGui()
{
    QFETCH(Qt::ToolButtonStyle, toolButtonStyle);
    QFETCH(Qt::ToolButtonStyle, expectedStyleMainToolbar);
    QFETCH(Qt::ToolButtonStyle, expectedStyleOtherToolbar);
    QFETCH(Qt::ToolButtonStyle, expectedStyleCleanToolbar);
    const Qt::ToolButtonStyle mainToolBarDefaultStyle = Qt::ToolButtonTextBesideIcon; // was TextUnderIcon before KDE-4.4.0
    KConfig config("tst_KToolBar");
    KConfigGroup group(&config, "group");
    {
        TestXmlGuiWindow kmw(m_xml);
        kmw.createActions(QStringList() << "go_up");
        kmw.createGUI();
        KToolBar* mainToolBar = kmw.toolBarByName("mainToolBar");
        KToolBar* otherToolBar = kmw.toolBarByName("otherToolBar");
        KToolBar* cleanToolBar = kmw.toolBarByName("cleanToolBar");

        QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)mainToolBarDefaultStyle);
        QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)Qt::ToolButtonTextUnderIcon); // from xml
        QCOMPARE((int)cleanToolBar->toolButtonStyle(), (int)Qt::ToolButtonTextBesideIcon);

        // Changing settings for a given toolbar, as user
        mainToolBar->setToolButtonStyle(toolButtonStyle);
        otherToolBar->setToolButtonStyle(toolButtonStyle);

        // Save settings
        kmw.saveMainWindowSettings(group);

        // Now change KDE-global setting
        changeGlobalToolButtonStyleSetting("IconOnly", "TextOnly");

        QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)expectedStyleMainToolbar);
        QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)expectedStyleOtherToolbar);
        QCOMPARE((int)cleanToolBar->toolButtonStyle(), (int)expectedStyleCleanToolbar);

    }
}

void tst_KToolBar::changeGlobalToolButtonStyleSetting(const QString& mainToolBar, const QString& otherToolBars)
{
    KConfigGroup group(KSharedConfig::openConfig(), "Toolbar style");
    group.writeEntry("ToolButtonStyle", mainToolBar);
    group.writeEntry("ToolButtonStyleOtherToolbars", otherToolBars);
    KSharedConfig::openConfig()->sync();
    // KGlobalSettings::emitChange(KGlobalSettings::ToolbarStyleChanged);
    // too racy: QEventLoop().processEvents( QEventLoop::AllEvents, 20 ); // need to process DBUS signal
    QMetaObject::invokeMethod(KGlobalSettings::self(), "_k_slotNotifyChange", Q_ARG(int, KGlobalSettings::ToolbarStyleChanged), Q_ARG(int, 0));
}

void tst_KToolBar::deleteGlobalToolButtonStyleSetting()
{
    KConfigGroup group(KSharedConfig::openConfig(), "Toolbar style");
    group.deleteEntry("ToolButtonStyle");
    group.deleteEntry("ToolButtonStyleOtherToolbars");
    KSharedConfig::openConfig()->sync();
    QMetaObject::invokeMethod(KGlobalSettings::self(), "_k_slotNotifyChange", Q_ARG(int, KGlobalSettings::ToolbarStyleChanged), Q_ARG(int, 0));
}

void tst_KToolBar::testToolBarPosition()
{
    TestXmlGuiWindow kmw(m_xml);
    kmw.createActions(QStringList() << "go_up");
    kmw.createGUI();
    KToolBar* mainToolBar = kmw.toolBarByName("mainToolBar");
    KToolBar* otherToolBar = kmw.toolBarByName("otherToolBar");
    QCOMPARE(kmw.toolBarArea(mainToolBar), Qt::TopToolBarArea);
    QCOMPARE(kmw.toolBarArea(otherToolBar), Qt::BottomToolBarArea);
}

void tst_KToolBar::testXmlGuiSwitching()
{
    const QByteArray windowXml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        "</MenuBar>\n"
        "</gui>\n";
    TestXmlGuiWindow kmw(windowXml);
    kmw.createActions(QStringList() << "go_up");
    kmw.createGUI();
    TestGuiClient firstClient(m_xml);
    kmw.guiFactory()->addClient(&firstClient);

    {
        //kDebug() << "Added gui client";
        KToolBar* mainToolBar = firstClient.toolBarByName("mainToolBar");
        KToolBar* otherToolBar = firstClient.toolBarByName("otherToolBar");
        KToolBar* bigToolBar = firstClient.toolBarByName("bigToolBar");
        KToolBar* hiddenToolBar = firstClient.toolBarByName("hiddenToolBar");
        KToolBar* secondHiddenToolBar = firstClient.toolBarByName("secondHiddenToolBar");
        QCOMPARE(hiddenToolBar->isHidden(), true);
        QCOMPARE(secondHiddenToolBar->isHidden(), true);
        // Make (unsaved) changes as user
        QMetaObject::invokeMethod(mainToolBar, "slotContextTextRight"); // mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        QMetaObject::invokeMethod(mainToolBar, "slotContextRight"); // kmw.addToolBar(Qt::RightToolBarArea, mainToolBar);
        otherToolBar->setIconDimensions(35);
        bigToolBar->setIconDimensions(35);
        bigToolBar->hide();
        hiddenToolBar->show();
    }
    kmw.guiFactory()->removeClient(&firstClient);
    //kDebug() << "Removed gui client";
    QVERIFY(!kmw.guiFactory()->container("mainToolBar", &kmw));
    QVERIFY(!kmw.guiFactory()->container("otherToolBar", &kmw));
    QVERIFY(!kmw.guiFactory()->container("bigToolBar", &kmw));
    QVERIFY(!kmw.guiFactory()->container("mainToolBar", &firstClient));
    QVERIFY(!kmw.guiFactory()->container("otherToolBar", &firstClient));
    QVERIFY(!kmw.guiFactory()->container("bigToolBar", &firstClient));

    kmw.guiFactory()->addClient(&firstClient);
    //kDebug() << "Re-added gui client";
    KToolBar* mainToolBar = firstClient.toolBarByName("mainToolBar");
    KToolBar* otherToolBar = firstClient.toolBarByName("otherToolBar");
    KToolBar* bigToolBar = firstClient.toolBarByName("bigToolBar");
    KToolBar* cleanToolBar = firstClient.toolBarByName("cleanToolBar");
    KToolBar* hiddenToolBar = firstClient.toolBarByName("hiddenToolBar");
    KToolBar* secondHiddenToolBar = firstClient.toolBarByName("secondHiddenToolBar");
    QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)Qt::ToolButtonTextBesideIcon);
    QCOMPARE(mainToolBar->isHidden(), false);
    QCOMPARE(kmw.toolBarArea(mainToolBar), Qt::RightToolBarArea);
    QCOMPARE(mainToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::MainToolbar));
    QCOMPARE(otherToolBar->iconSize().width(), 35);
    QCOMPARE(bigToolBar->iconSize().width(), 35);
    QCOMPARE(cleanToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::Toolbar));
    QCOMPARE(bigToolBar->isHidden(), true);
    QCOMPARE(hiddenToolBar->isHidden(), false);
    QCOMPARE(secondHiddenToolBar->isHidden(), true);

    // Now change KDE-global setting, what happens to unsaved changes?
    changeGlobalIconSizeSetting(32, 33);
    QCOMPARE(bigToolBar->iconSize().width(), 35); // fine now, saved or unsaved makes no difference
    QCOMPARE(otherToolBar->iconSize().width(), 35);

    // Now save, and check what we saved
    KConfig config("tst_KToolBar");
    KConfigGroup group(&config, "group");
    kmw.saveMainWindowSettings(group);
    QCOMPARE(group.group("Toolbar bigToolBar").readEntry("IconSize", 0), 35);
    QCOMPARE(group.group("Toolbar otherToolBar").readEntry("IconSize", 0), 35);
    QVERIFY(!group.group("Toolbar cleanToolBar").hasKey("IconSize"));
    //QCOMPARE(group.group("Toolbar bigToolBar").readEntry("Hidden", false), true);
    //QVERIFY(!group.group("Toolbar cleanToolBar").hasKey("Hidden"));
    //QVERIFY(!group.group("Toolbar hiddenToolBar").hasKey("Hidden"));

    // Recreate window and apply config; is hidden toolbar shown as expected?
    {
        TestXmlGuiWindow kmw2(windowXml);
        kmw2.createActions(QStringList() << "go_up");
        kmw2.createGUI();
        TestGuiClient firstClient(m_xml);
        kmw2.guiFactory()->addClient(&firstClient);

        KToolBar* mainToolBar = firstClient.toolBarByName("mainToolBar");
        KToolBar* otherToolBar = firstClient.toolBarByName("otherToolBar");
        KToolBar* bigToolBar = firstClient.toolBarByName("bigToolBar");
        KToolBar* hiddenToolBar = firstClient.toolBarByName("hiddenToolBar");
        KToolBar* secondHiddenToolBar = firstClient.toolBarByName("secondHiddenToolBar");
        QCOMPARE(bigToolBar->isHidden(), false);
        QCOMPARE(hiddenToolBar->isHidden(), true);
        QCOMPARE(secondHiddenToolBar->isHidden(), true);

        kmw2.show();

        // Check that secondHiddenToolBar is not shown+hidden immediately?
        m_showWasCalled = false;
        secondHiddenToolBar->installEventFilter(this);
        kmw2.applyMainWindowSettings(group);

        QCOMPARE(mainToolBar->isHidden(), false);
        QCOMPARE(kmw2.toolBarArea(mainToolBar), Qt::RightToolBarArea);
        QCOMPARE(otherToolBar->iconSize().width(), 35);
        QCOMPARE(bigToolBar->iconSize().width(), 35);
        QCOMPARE(bigToolBar->isHidden(), true);
        QCOMPARE(hiddenToolBar->isHidden(), false);
        QCOMPARE(secondHiddenToolBar->isHidden(), true);
        QVERIFY(!m_showWasCalled);
    }
}

void tst_KToolBar::testClickingToolButton_data() {
    QTest::addColumn<Qt::MouseButton>("button");
    QTest::addColumn<Qt::KeyboardModifiers>("modifiers");

    // Note that Qt::NoModifier, Qt::ShiftModifier, etc. are of the type Qt::KeyboardModifier.
    // If they are not combined with other modifiers, they must be cast explicitly to
    // Qt::KeyboardModifiers to make the test work.

    QTest::newRow("LMB") << Qt::LeftButton << Qt::KeyboardModifiers(Qt::NoModifier);
    QTest::newRow("Shift+LMB") << Qt::LeftButton << Qt::KeyboardModifiers(Qt::ShiftModifier);
    QTest::newRow("Ctrl+LMB") << Qt::LeftButton << Qt::KeyboardModifiers(Qt::ControlModifier);
    QTest::newRow("Alt+LMB") << Qt::LeftButton << Qt::KeyboardModifiers(Qt::AltModifier);
    QTest::newRow("Shift+Control+LMB") << Qt::LeftButton << (Qt::ShiftModifier | Qt::ControlModifier);
    QTest::newRow("Shift+Alt+LMB") << Qt::LeftButton << (Qt::ShiftModifier | Qt::AltModifier);
    QTest::newRow("Control+Alt+LMB") << Qt::LeftButton << (Qt::ControlModifier | Qt::AltModifier);
    QTest::newRow("Shift+Control+Alt+LMB") << Qt::LeftButton << (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier);

    QTest::newRow("MMB") << Qt::MidButton << Qt::KeyboardModifiers(Qt::NoModifier);
    QTest::newRow("Shift+MMB") << Qt::MidButton << Qt::KeyboardModifiers(Qt::ShiftModifier);
    QTest::newRow("Ctrl+MMB") << Qt::MidButton << Qt::KeyboardModifiers(Qt::ControlModifier);
    QTest::newRow("Alt+MMB") << Qt::MidButton << Qt::KeyboardModifiers(Qt::AltModifier);
    QTest::newRow("Shift+Control+MMB") << Qt::MidButton << (Qt::ShiftModifier | Qt::ControlModifier);
    QTest::newRow("Shift+Alt+MMB") << Qt::MidButton << (Qt::ShiftModifier | Qt::AltModifier);
    QTest::newRow("Control+Alt+MMB") << Qt::MidButton << (Qt::ControlModifier | Qt::AltModifier);
    QTest::newRow("Shift+Control+Alt+MMB") << Qt::MidButton << (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier);
}

void tst_KToolBar::testClickingToolButton() {
    KMainWindow mainWindow;
    KToolBar toolBar(&mainWindow);
    KAction* testAction = new KAction("test", &toolBar);
    QSignalSpy spy(testAction, SIGNAL(triggered()));
    QSignalSpy spyButtonsModifiers(testAction, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)));
    toolBar.addAction(testAction);
    QToolButton* testToolButton = qobject_cast<QToolButton*>(toolBar.widgetForAction(testAction));
    QVERIFY(testToolButton);

    QFETCH(Qt::MouseButton, button);
    QFETCH(Qt::KeyboardModifiers, modifiers);

    // Must convert button (which is of the type Qt::MouseButton) to Qt::MouseButtons to make the tests below work
    Qt::MouseButtons pressedButtons = button;

    // Click the button in the toolbar
    QTest::mouseClick(testToolButton, button, modifiers);
    QCOMPARE(spyButtonsModifiers.count(), 1);
    QList<QVariant> arguments = spyButtonsModifiers.takeFirst();
    // This needs Qt5 MR 43915, i.e. Qt >= 5.0.1.
    // But let's only enable it when 5.1 comes around; the currently outdated Qt dev branch breaks otherwise.
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
    if(button != Qt::LeftButton) {
        // Only check the "buttons" argument of KAction's triggered(Qt::MouseButtons, Qt::KeyboardModifiers)
        // signal if the middle mouse button is pressed. If the left button is pressed, "buttons" is
        // Qt::NoButton, i.e., incorrect. The reason is that it is determined in KActionPrivate::slotTriggered()
        // using QApplication::mouseButtons() _after_ the button has been released.
        QCOMPARE(arguments.at(0).value<Qt::MouseButtons>(), pressedButtons);
    }
    QCOMPARE(arguments.at(1).value<Qt::KeyboardModifiers>(), modifiers);
#endif

    if(button == Qt::LeftButton) {
        // Also the action's triggered() signal without arguments should have been emitted in this case
        QCOMPARE(spy.count(), 1);
        spy.takeFirst();
    }
    else {
        QCOMPARE(spy.count(), 0);
    }

    // Disable the action, press and release the button again.
    // Verify that the button is not "down" and that the signals are not emitted.
    testAction->setEnabled(false);
    QTest::mousePress(testToolButton, button, modifiers);
    QVERIFY(!testToolButton->isDown());
    QTest::mouseRelease(testToolButton, button, modifiers);
    QCOMPARE(spyButtonsModifiers.count(), 0);
    QCOMPARE(spy.count(), 0);

    // Check that it also works fine if the action is disabled while the button is "down":
    // Enable the action, press the button, disable the action and release.
    // Verify that the "down" state is as expected and that the signals are not emitted.
    testAction->setEnabled(true);
    QTest::mousePress(testToolButton, button, modifiers);
    QVERIFY(testToolButton->isDown());
    testAction->setEnabled(false);
    QTest::mouseRelease(testToolButton, button, modifiers);
    QVERIFY(!testToolButton->isDown());
    QCOMPARE(spyButtonsModifiers.count(), 0);
    QCOMPARE(spy.count(), 0);
}

bool tst_KToolBar::eventFilter(QObject * watched, QEvent * event)
{
    Q_UNUSED(watched);
    if (event->type() == QEvent::Show) {
        m_showWasCalled = true;
        return true;
    }
    return false;
}

#include "ktoolbar_unittest.moc"
