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
#include "../src/platformtheme/kdeplatformtheme.h"

#include <Qt>
#include <QTest>
#include <QDir>
#include <QFile>
#include <QString>
#include <QDialogButtonBox>

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

class KdePlatformTheme_UnitTest : public QObject
{
    Q_OBJECT
    private Q_SLOTS:
        void testPlatformHints()
        {
            KdePlatformTheme *qpa = new KdePlatformTheme();
            QCOMPARE(qpa->themeHint(QPlatformTheme::CursorFlashTime).toInt(), 1042);
            QCOMPARE(qpa->themeHint(QPlatformTheme::MouseDoubleClickInterval).toInt(), 4343);
            QCOMPARE(qpa->themeHint(QPlatformTheme::StartDragDistance).toInt(), 15);
            QCOMPARE(qpa->themeHint(QPlatformTheme::StartDragTime).toInt(), 555);
            QCOMPARE(qpa->themeHint(QPlatformTheme::ToolButtonStyle).toInt(), (int) Qt::ToolButtonTextOnly);
            QCOMPARE(qpa->themeHint(QPlatformTheme::ToolBarIconSize).toInt(), 2);
            QCOMPARE(qpa->themeHint(QPlatformTheme::ItemViewActivateItemOnSingleClick).toBool(), false);
            QCOMPARE(qpa->themeHint(QPlatformTheme::SystemIconThemeName).toString(), QLatin1String("non-existent-icon-theme"));
            QCOMPARE(qpa->themeHint(QPlatformTheme::SystemIconFallbackThemeName).toString(), QLatin1String("hicolor"));
            QVERIFY(qpa->themeHint(QPlatformTheme::IconThemeSearchPaths).toString().isEmpty());
            QStringList styles;
            styles << "non-existent-widget-style" << "oxygen" << "fusion" << "windows";
            QCOMPARE(qpa->themeHint(QPlatformTheme::StyleNames).toStringList(), styles);
            QCOMPARE(qpa->themeHint(QPlatformTheme::DialogButtonBoxLayout).toInt(), (int) QDialogButtonBox::KdeLayout);
            QCOMPARE(qpa->themeHint(QPlatformTheme::DialogButtonBoxButtonsHaveIcons).toBool(), false);
            QCOMPARE(qpa->themeHint(QPlatformTheme::UseFullScreenForPopupMenu).toBool(), true);
            QCOMPARE(qpa->themeHint(QPlatformTheme::KeyboardScheme).toInt(), (int) QPlatformTheme::KdeKeyboardScheme);
            QCOMPARE(qpa->themeHint(QPlatformTheme::UiEffects).toInt(), 0);
            QCOMPARE(qpa->themeHint(QPlatformTheme::IconPixmapSizes).value<QList<int> >(), QList<int>() << 512 << 256 << 128 << 64 << 32 << 22 << 16 << 8);
        }
};

QTEST_MAIN(KdePlatformTheme_UnitTest)

#include "kdeplatformtheme_unittest.moc"