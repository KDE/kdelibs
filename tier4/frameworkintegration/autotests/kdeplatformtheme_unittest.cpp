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
#include "../src/platformtheme/khintssettings.h"

#include <Qt>
#include <QTest>
#include <QDir>
#include <QFile>
#include <QString>
#include <QPalette>
#include <QDebug>
#include <QIconEngine>
#include <QToolButton>
#include <QApplication>
#include <QDialogButtonBox>
#include <QStandardPaths>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>

#include <kiconloader.h>

static void prepareEnvironment()
{
    QStandardPaths::setTestModeEnabled(true);

    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    configPath.append("/kdeglobals");

    QFile::remove(configPath);
    QFile::copy(CONFIGFILE, configPath);
}

Q_COREAPP_STARTUP_FUNCTION(prepareEnvironment);

class EventTest : public QObject
{
public:
    EventTest(QObject* tested, QEvent::Type type)
        : QObject(), gotEvent(false), m_type(type)
    {
        tested->installEventFilter(this);

    }

    virtual bool eventFilter(QObject*, QEvent* e)
    {
        if (e->type() == m_type) {
            gotEvent = true;
        }
        return false;
    }

    bool gotEvent;
    QEvent::Type m_type;
};

class KdePlatformTheme_UnitTest : public QObject
{
    Q_OBJECT
    public:
        KdePlatformTheme_UnitTest()
        {}

    private:
        void sendNotifyChange(KHintsSettings::ChangeType type, int arg = -1)
        {
            QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange" );
            QList<QVariant> args;
            args.append(static_cast<int>(type));
            if (arg >= 0) {
                args.append(arg);
            }
            message.setArguments(args);
            QDBusConnection::sessionBus().send(message);
        }

        QEventLoop m_loop;
        QToolButton m_toolBtn;
        KdePlatformTheme *m_qpa;
    private Q_SLOTS:
        void initTestCase()
        {
            m_qpa = new KdePlatformTheme();
            QDBusConnection::sessionBus().connect(QString(), "/KGlobalSettings", "org.kde.KGlobalSettings",
                                                   "notifyChange",  &m_loop, SLOT(quit()));
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

            QCOMPARE(qApp->wheelScrollLines(), 1234);
            QCOMPARE(qApp->testAttribute(Qt::AA_DontShowIconsInMenus), false);
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
            QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
            configPath.append("/kdeglobals");
            QFile::remove(configPath);
            QFile::copy(CHANGED_CONFIGFILE, configPath);

            QDBusConnection::sessionBus().connect(QString(), "/KIconLoader", "org.kde.KIconLoader",
                                                   "iconChanged",  &m_loop, SLOT(quit()));

            QDBusMessage message = QDBusMessage::createSignal("/KIconLoader", "org.kde.KIconLoader", "iconChanged" );
            message.setArguments(QList<QVariant>() << int(KIconLoader::MainToolbar));
            QDBusConnection::sessionBus().send(message);
            m_loop.exec();

            QCOMPARE(m_qpa->themeHint(QPlatformTheme::ToolBarIconSize).toInt(), 11);
        }

        void testPlatformHintChanges()
        {
            EventTest tester(&m_toolBtn, QEvent::StyleChange);
            sendNotifyChange(KHintsSettings::SettingsChanged, KHintsSettings::SETTINGS_QT);
            m_loop.exec();

            QCOMPARE(m_qpa->themeHint(QPlatformTheme::CursorFlashTime).toInt(), 1022);

            sendNotifyChange(KHintsSettings::SettingsChanged, KHintsSettings::SETTINGS_MOUSE);
            m_loop.exec();

            QCOMPARE(m_qpa->themeHint(QPlatformTheme::ItemViewActivateItemOnSingleClick).toBool(), true);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::MouseDoubleClickInterval).toInt(), 401);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::StartDragDistance).toInt(), 35);
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::StartDragTime).toInt(), 501);

            QCOMPARE(qApp->wheelScrollLines(), 122);
            QCOMPARE(qApp->testAttribute(Qt::AA_DontShowIconsInMenus), true);

            sendNotifyChange(KHintsSettings::ToolbarStyleChanged, 2);
            m_loop.exec();

            QCOMPARE(m_qpa->themeHint(QPlatformTheme::ToolButtonStyle).toInt(), (int) Qt::ToolButtonTextUnderIcon);
            QCOMPARE(tester.gotEvent, true);

            sendNotifyChange(KHintsSettings::StyleChanged, 2);
            m_loop.exec();

            QStringList styles;
            styles << "another-non-existent-widget-style" << "oxygen" << "fusion" << "windows";
            QCOMPARE(m_qpa->themeHint(QPlatformTheme::StyleNames).toStringList(), styles);

            sendNotifyChange(KHintsSettings::SettingsChanged, KHintsSettings::SETTINGS_STYLE);
            m_loop.exec();

            QCOMPARE(m_qpa->themeHint(QPlatformTheme::DialogButtonBoxButtonsHaveIcons).toBool(), true);

            sendNotifyChange(KHintsSettings::IconChanged, 4);
            m_loop.exec();

            QCOMPARE(m_qpa->themeHint(QPlatformTheme::SystemIconThemeName).toString(), QLatin1String("other-non-existent"));
        }

        void testPlatformPaletteChanges()
        {
            EventTest tester(QGuiApplication::instance(), QEvent::ApplicationPaletteChange);
            sendNotifyChange(KHintsSettings::PaletteChanged, 0);
            m_loop.exec();
            QCOMPARE(tester.gotEvent, true);

            const QPalette *palette = m_qpa->palette();
            QPalette::ColorGroup states[3] = {QPalette::Active, QPalette::Inactive, QPalette::Disabled};
            QColor redColor(QColor(174,11,11));
            QBrush redBrush(redColor);
            for ( int i = 0; i < 3 ; i++ ) {
                QCOMPARE(palette->brush(states[i], QPalette::ButtonText), redBrush);
                QCOMPARE(palette->brush(states[i], QPalette::WindowText), redBrush);
                QCOMPARE(palette->brush(states[i], QPalette::Window), redBrush);
                QCOMPARE(palette->brush(states[i], QPalette::Base), redBrush);
                QCOMPARE(palette->brush(states[i], QPalette::Text), redBrush);
                QCOMPARE(palette->brush(states[i], QPalette::Button), redBrush);
                QCOMPARE(palette->brush(states[i], QPalette::ButtonText), redBrush);
                QCOMPARE(palette->brush(states[i], QPalette::Highlight), redBrush);
                QCOMPARE(palette->brush(states[i], QPalette::HighlightedText), redBrush);
                QCOMPARE(palette->brush(states[i], QPalette::ToolTipBase), redBrush);
                QCOMPARE(palette->brush(states[i], QPalette::ToolTipText), redBrush);

                //KColorScheme applies modifications and we can't disable them, so I extracted
                //the values and blindly compare them.
                QCOMPARE(palette->color(states[i], QPalette::Light).red(), 230);
                QCOMPARE(palette->color(states[i], QPalette::Midlight).red(), 203);
                QCOMPARE(palette->color(states[i], QPalette::Mid).red(), 149);
                QCOMPARE(palette->color(states[i], QPalette::Dark).red(), 84);
                QCOMPARE(palette->color(states[i], QPalette::Shadow).red(), 60);

                QCOMPARE(palette->brush(states[i], QPalette::AlternateBase), redBrush);
                QCOMPARE(palette->brush(states[i], QPalette::Link), redBrush);
                QCOMPARE(palette->brush(states[i], QPalette::LinkVisited), redBrush);
            }
        }
};

QTEST_MAIN(KdePlatformTheme_UnitTest)

#include "kdeplatformtheme_unittest.moc"
