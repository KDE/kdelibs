/*  This file is part of the KDE libraries
 *  Copyright 2013 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kdeplatformtheme_config.h"
#include "../src/platformtheme/kfontsettingsdata.h"

#include <Qt>
#include <QTest>
#include <QDir>
#include <QFile>
#include <QString>
#include <QPalette>
#include <QIconEngine>
#include <QApplication>
#include <QDialogButtonBox>

#include <QDBusMessage>
#include <QDBusConnection>

#include <QDebug>

static void prepareEnvironment()
{
    qputenv("KDEHOME", QFile::encodeName(QDir::homePath() + QStringLiteral("/.kde5-unit-test-platformtheme")));
    qputenv("XDG_DATA_HOME", QFile::encodeName(QDir::homePath() + QStringLiteral("/.kde5-unit-test-platformtheme/xdg/local")));
    QByteArray configPath = QFile::encodeName(QDir::homePath() + QStringLiteral("/.kde5-unit-test-platformtheme/xdg/config"));
    qputenv("XDG_CONFIG_HOME", configPath);
    qputenv("KDE_SKIP_KDERC", "1");
    qunsetenv("KDE_COLOR_DEBUG");

    QDir().mkpath(configPath);
    configPath.append("/kdeglobals");
    QFile::remove(configPath);
    QFile::copy(CONFIGFILE, configPath);
}

Q_COREAPP_STARTUP_FUNCTION(prepareEnvironment);

class KFontSettingsData_UnitTest : public QWidget
{
    Q_OBJECT
    public:
        virtual bool event(QEvent* e)
        {
            if (e->type() == QEvent::ApplicationFontChange) {
                m_appChangedFont = true;
            }
            return QWidget::event(e);
        }
    private:
        bool m_appChangedFont;
        KFontSettingsData *m_fonts;
    private Q_SLOTS:
        void initTestCase()
        {
            m_appChangedFont = false;
            m_fonts = new KFontSettingsData;
        }

        void testFonts()
        {
            QCOMPARE(m_fonts->font(KFontSettingsData::GeneralFont)->family(), QStringLiteral("OxyFontTest"));
            QCOMPARE(m_fonts->font(KFontSettingsData::FixedFont)->family(), QStringLiteral("OxyFixedTest Mono"));
            QCOMPARE(m_fonts->font(KFontSettingsData::ToolbarFont)->family(), QStringLiteral("OxyToolbarTest"));
            QCOMPARE(m_fonts->font(KFontSettingsData::MenuFont)->family(), QStringLiteral("OxyMenuTest"));
            QCOMPARE(m_fonts->font(KFontSettingsData::WindowTitleFont)->family(), QStringLiteral("OxyActiveTest"));
            QCOMPARE(m_fonts->font(KFontSettingsData::TaskbarFont)->family(), QStringLiteral("OxyTaskbarTest"));
            QCOMPARE(m_fonts->font(KFontSettingsData::SmallestReadableFont)->family(), QStringLiteral("OxySmallestReadableTest"));
        }

        void testFontsChanged()
        {
            QByteArray configPath = qgetenv("XDG_CONFIG_HOME");
            configPath.append("/kdeglobals");
            QFile::remove(configPath);
            QFile::copy(CHANGED_CONFIGFILE, configPath);

            QEventLoop loop;
            QDBusConnection::sessionBus().connect( QString(), "/KDEPlatformTheme", "org.kde.KDEPlatformTheme",
                                                      "refreshFonts", &loop, SLOT(quit()) );

            QDBusMessage message = QDBusMessage::createSignal("/KDEPlatformTheme", "org.kde.KDEPlatformTheme", "refreshFonts" );
            QDBusConnection::sessionBus().send(message);
            loop.exec();

            QVERIFY(m_appChangedFont);
            QCOMPARE(m_fonts->font(KFontSettingsData::GeneralFont)->family(), QStringLiteral("ChangedFontTest"));
            QCOMPARE(m_fonts->font(KFontSettingsData::FixedFont)->family(), QStringLiteral("ChangedFixedTest Mono"));
            QCOMPARE(m_fonts->font(KFontSettingsData::ToolbarFont)->family(), QStringLiteral("ChangedToolbarTest"));
            QCOMPARE(m_fonts->font(KFontSettingsData::MenuFont)->family(), QStringLiteral("ChangedMenuTest"));
            QCOMPARE(m_fonts->font(KFontSettingsData::WindowTitleFont)->family(), QStringLiteral("ChangedActiveTest"));
            QCOMPARE(m_fonts->font(KFontSettingsData::TaskbarFont)->family(), QStringLiteral("ChangedTaskbarTest"));
            QCOMPARE(m_fonts->font(KFontSettingsData::SmallestReadableFont)->family(), QStringLiteral("ChangedSmallestReadableTest"));
        }
};

QTEST_MAIN(KFontSettingsData_UnitTest)

#include "kfontsettingsdata_unittest.moc"