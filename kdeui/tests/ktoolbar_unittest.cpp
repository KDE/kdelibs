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
#include <kstandarddirs.h>
#include <QtTest/QtTest>
#include <qtest_kde.h>
#include <ktoolbar.h>
#include <kmainwindow.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <QDomElement>
#include "testxmlguiwindow.h"

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
    void testToolButtonStyleNoXmlGui();

private:
    void changeGlobalIconSizeSetting(int, int);
    void deleteGlobalIconSizeSetting();
    QByteArray m_xml;
};

QTEST_KDEMAIN(tst_KToolBar, GUI)

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
        "<ToolBar name=\"otherToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"cleanToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar iconSize=\"32\" name=\"bigToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar iconSize=\"32\" name=\"bigUnchangedToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "</gui>\n";
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
    QFile::remove(KStandardDirs::locateLocal("config", "tst_KToolBar"));
    deleteGlobalIconSizeSetting();
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
        QCOMPARE(group.groupList().count(), 5); // one subgroup for each toolbar
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

Q_DECLARE_METATYPE(Qt::ToolButtonStyle)

void tst_KToolBar::testToolButtonStyleNoXmlGui()
{
    KConfig config("tst_KToolBar");
    KConfigGroup group(&config, "group");
    {
        KMainWindow kmw;
        KToolBar* mainToolBar = kmw.toolBar("mainToolBar");
        KToolBar* otherToolBar = kmw.toolBar("otherToolBar");

        // Default settings (applied by applyAppearanceSettings)
        QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)Qt::ToolButtonTextUnderIcon);
        QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)Qt::ToolButtonTextBesideIcon); // see r883541
        QCOMPARE(kmw.toolBarArea(mainToolBar), Qt::TopToolBarArea);

        // Changing settings for a given toolbar, as user
        mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        otherToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

        // Save settings
        kmw.saveMainWindowSettings(group);
        QCOMPARE(group.groupList().count(), 2); // two subgroups (one for each toolbar)
        QVERIFY(group.group("Toolbar mainToolBar").hasKey("ToolButtonStyle"));
    }

    {
        // Recreate, load, compare.
        KMainWindow kmw;
        KToolBar* mainToolBar = kmw.toolBar("mainToolBar");
        KToolBar* otherToolBar = kmw.toolBar("otherToolBar");
        QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)Qt::ToolButtonTextUnderIcon);
        kmw.applyMainWindowSettings(group);
        QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)Qt::ToolButtonIconOnly);
        QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)Qt::ToolButtonIconOnly);

        // Set the default value manually again
        mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        otherToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        kmw.saveMainWindowSettings(group);
        // Check that the keys got deleted from the config file
        QVERIFY(!group.group("Toolbar mainToolBar").hasKey("ToolButtonStyle"));
    }

    // TODO: changing KDE-global setting, see KToolBar::toolButtonStyleSetting()
}

void tst_KToolBar::changeGlobalIconSizeSetting(int mainToolbarIconSize, int iconSize)
{
    // We could use KConfig::Normal|KConfig::Global here, to write to kdeglobals like kcmstyle does,
    // but we don't need to. Writing to the app's config file works too.
    KConfigGroup mglobals(KGlobal::config(), "MainToolbarIcons");
    mglobals.writeEntry("Size", mainToolbarIconSize);
    KConfigGroup globals(KGlobal::config(), "ToolbarIcons");
    //globals.writeEntry("Size", iconSize, KConfig::Normal|KConfig::Global);
    globals.writeEntry("Size", iconSize);
    KGlobal::config()->sync();
    // KGlobalSettings::emitChange(KGlobalSettings::ToolbarStyleChanged);
    // too racy: QEventLoop().processEvents( QEventLoop::AllEvents, 20 ); // need to process DBUS signal
    //QMetaObject::invokeMethod(KGlobalSettings::self(), "_k_slotNotifyChange", Q_ARG(int, KGlobalSettings::ToolbarStyleChanged), Q_ARG(int, 0));
    QMetaObject::invokeMethod(KGlobalSettings::self(), "_k_slotNotifyChange", Q_ARG(int, KGlobalSettings::IconChanged), Q_ARG(int, 0));
}

void tst_KToolBar::deleteGlobalIconSizeSetting()
{
    KConfigGroup mglobals(KGlobal::config(), "MainToolbarIcons");
    mglobals.deleteEntry("Size");
    KConfigGroup globals(KGlobal::config(), "ToolbarIcons");
    globals.deleteEntry("Size");
    KGlobal::config()->sync();
    //KGlobalSettings::emitChange(KGlobalSettings::ToolbarStyleChanged);
    //QEventLoop().processEvents( QEventLoop::AllEvents, 20 ); // need to process DBUS signal
    QMetaObject::invokeMethod(KGlobalSettings::self(), "_k_slotNotifyChange", Q_ARG(int, KGlobalSettings::IconChanged), Q_ARG(int, 0));
}

#include "ktoolbar_unittest.moc"
