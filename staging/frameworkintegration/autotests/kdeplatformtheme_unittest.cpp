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
#include <QPalette>
#include <QIconEngine>
#include <QDialogButtonBox>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>

#include <kiconloader.h>
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
    private:
        QEventLoop m_loop;
        KdePlatformTheme *m_qpa;
    private Q_SLOTS:
        void initTestCase()
        {
            m_qpa = new KdePlatformTheme();
        }

        void testPlatformHints()
        {
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::CursorFlashTime).toInt(), 1042);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::MouseDoubleClickInterval).toInt(), 4343);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::StartDragDistance).toInt(), 15);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::StartDragTime).toInt(), 555);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::ToolButtonStyle).toInt(), (int) Qt::ToolButtonTextOnly);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::ToolBarIconSize).toInt(), 2);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::ItemViewActivateItemOnSingleClick).toBool(), false);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::SystemIconThemeName).toString(), QLatin1String("non-existent-icon-theme"));
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::SystemIconFallbackThemeName).toString(), QLatin1String("hicolor"));
            QVERIFY(m_qpa->themeHint(QPlatformTheme::IconThemeSearchPaths).toString().isEmpty());
            QStringList styles;
            styles << "non-existent-widget-style" << "oxygen" << "fusion" << "windows";
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::StyleNames).toStringList(), styles);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::DialogButtonBoxLayout).toInt(), (int) QDialogButtonBox::KdeLayout);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::DialogButtonBoxButtonsHaveIcons).toBool(), false);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::UseFullScreenForPopupMenu).toBool(), true);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::KeyboardScheme).toInt(), (int) QPlatformTheme::KdeKeyboardScheme);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::UiEffects).toInt(), 0);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::IconPixmapSizes).value<QList<int> >(), QList<int>() << 512 << 256 << 128 << 64 << 32 << 22 << 16 << 8);
        }

        void testPlatformPalette()
        {
            const QPalette *palette = m_qpa->palette();
            QPalette::ColorGroup states[3] = {QPalette::Active, QPalette::Inactive, QPalette::Disabled};
            QColor greenColor(QColor(0, 128,0));
            QBrush greenBrush(greenColor);
            for ( int i = 0; i < 3 ; i++ ) {
                QCOMPARE(palette->brush(states[i], QPalette::ButtonText), greenBrush);
                QCOMPARE(palette->brush(states[i], QPalette::WindowText), greenBrush);
                QCOMPARE(palette->brush(states[i], QPalette::Window), greenBrush);
                QCOMPARE(palette->brush(states[i], QPalette::Base), greenBrush);
                QCOMPARE(palette->brush(states[i], QPalette::Text), greenBrush);
                QCOMPARE(palette->brush(states[i], QPalette::Button), greenBrush);
                QCOMPARE(palette->brush(states[i], QPalette::ButtonText), greenBrush);
                QCOMPARE(palette->brush(states[i], QPalette::Highlight), greenBrush);
                QCOMPARE(palette->brush(states[i], QPalette::HighlightedText), greenBrush);
                QCOMPARE(palette->brush(states[i], QPalette::ToolTipBase), greenBrush);
                QCOMPARE(palette->brush(states[i], QPalette::ToolTipText), greenBrush);

                //KColorScheme applies modifications and we can't disable them, so I extracted
                //the values and blindly compare them.
                QCOMPARE(palette->color(states[i], QPalette::Light).green(), 162);
                QCOMPARE(palette->color(states[i], QPalette::Midlight).green(), 144);
                QCOMPARE(palette->color(states[i], QPalette::Mid).green(), 109);
                QCOMPARE(palette->color(states[i], QPalette::Dark).green(), 62);
                QCOMPARE(palette->color(states[i], QPalette::Shadow).green(), 43);

                QCOMPARE(palette->brush(states[i], QPalette::AlternateBase), greenBrush);
                QCOMPARE(palette->brush(states[i], QPalette::Link), greenBrush);
                QCOMPARE(palette->brush(states[i], QPalette::LinkVisited), greenBrush);
            }
        }

        void testPlatformIconEngine()
        {
            QIconEngine *engine = m_qpa->createIconEngine(QStringLiteral("test-icon"));
            QCOMPARE(engine->key(), QStringLiteral("KIconEngine"));
        }

        void testPlatformIconChanges()
        {
            QByteArray configPath = qgetenv("XDG_CONFIG_HOME");
            configPath.append("/kdeglobals");
            QFile::remove(configPath);
            QFile::copy(CHANGED_CONFIGFILE, configPath);

            QDBusConnection::sessionBus().connect(QString(), "/KIconLoader", "org.kde.KIconLoader",
                                                   "iconChanged",  &m_loop, SLOT(quit()));

            QDBusMessage message = QDBusMessage::createSignal("/KIconLoader", "org.kde.KIconLoader", "iconChanged" );
            message.setArguments(QList<QVariant>() << int(KIconLoader::Toolbar));
            QDBusConnection::sessionBus().send(message);
            m_loop.exec();

            QCOMPARE(m_qpa->themeHint(QPlatformTheme::ToolBarIconSize).toInt(), 11);
        }
};

QTEST_MAIN(KdePlatformTheme_UnitTest)

#include "kdeplatformtheme_unittest.moc"