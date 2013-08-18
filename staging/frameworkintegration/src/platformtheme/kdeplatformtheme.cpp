/*  This file is part of the KDE libraries
 *  Copyright 2013 Kevin Ottens <ervin+bluesystems@kde.org>
 *  Copyright 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
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

#include "kdeplatformtheme.h"
#include "kfontsettingsdata.h"

#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QPalette>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariant>

#include <QtWidgets/QApplication>
#include <QtWidgets/QToolBar>
#include <QMainWindow>

#include <kcolorscheme.h>
#include <kconfiggroup.h>
#include <kiconengine_p.h>
#include <kiconloader.h>
#include <ksharedconfig.h>

KdePlatformTheme::KdePlatformTheme()
{
    loadSettings();
}

KdePlatformTheme::~KdePlatformTheme()
{
    delete m_fontsData;
    qDeleteAll(m_palettes);
}

QVariant KdePlatformTheme::themeHint(QPlatformTheme::ThemeHint hint) const
{
    QHash<ThemeHint, QVariant>::const_iterator it = m_hints.constFind(hint);
    if (it != m_hints.constEnd())
        return *it;
    else
        return QPlatformTheme::themeHint(hint);
}

const QPalette *KdePlatformTheme::palette(Palette type) const
{
    QHash<Palette, QPalette*>::const_iterator it = m_palettes.constFind(type);
    if (it != m_palettes.constEnd())
        return *it;
    else
        return QPlatformTheme::palette(type);
}

const QFont *KdePlatformTheme::font(Font type) const
{
    KFontSettingsData::FontTypes fdtype;
    switch(type) {
        case SystemFont:
            fdtype = KFontSettingsData::GeneralFont; break;
        case MenuFont:
        case MenuBarFont:
        case MenuItemFont:
            fdtype = KFontSettingsData::MenuFont; break;
        case MessageBoxFont:
        case LabelFont:
        case TipLabelFont:
        case StatusBarFont:
        case PushButtonFont:
        case ToolButtonFont:
        case ItemViewFont:
        case ListViewFont:
        case HeaderViewFont:
        case ListBoxFont:
        case ComboMenuItemFont:
        case ComboLineEditFont:
            fdtype = KFontSettingsData::GeneralFont; break;
        case TitleBarFont:
        case MdiSubWindowTitleFont:
        case DockWidgetTitleFont:
            fdtype = KFontSettingsData::WindowTitleFont; break;
        case SmallFont:
        case MiniFont:
            fdtype = KFontSettingsData::SmallestReadableFont; break;
        default:
            fdtype = KFontSettingsData::GeneralFont; break;
    }

    return m_fontsData->font(fdtype);
}

QIconEngine *KdePlatformTheme::createIconEngine(const QString &iconName) const
{
    return new KIconEngine(iconName, KIconLoader::global());
}

QStringList xdgIconThemePaths()
{
    QStringList paths;

    const QFileInfo homeIconDir(QDir::homePath() + QStringLiteral("/.icons"));
    if (homeIconDir.isDir())
        paths << homeIconDir.absoluteFilePath();

    QString xdgDirString = QFile::decodeName(qgetenv("XDG_DATA_DIRS"));
    if (xdgDirString.isEmpty())
        xdgDirString = QLatin1String("/usr/local/share/:/usr/share/");

    foreach (const QString &xdgDir, xdgDirString.split(QLatin1Char(':'))) {
        const QFileInfo xdgIconsDir(xdgDir + QStringLiteral("/icons"));
        if (xdgIconsDir.isDir())
            paths << xdgIconsDir.absoluteFilePath();
    }

    return paths;
}

void KdePlatformTheme::loadSettings()
{
    loadHints();
    loadPalettes();

    m_fontsData = new KFontSettingsData;

    QTimer::singleShot(0, this, SLOT(setupIconLoader()));
}

void KdePlatformTheme::loadHints()
{
    KConfigGroup cg(KSharedConfig::openConfig("kdeglobals"), "KDE");

    m_hints[CursorFlashTime] = qBound(200, cg.readEntry("CursorBlinkRate", 1000), 2000);
    m_hints[MouseDoubleClickInterval] = cg.readEntry("DoubleClickInterval", 400);
    m_hints[StartDragDistance] = cg.readEntry("StartDragDist", 10);
    m_hints[StartDragTime] = cg.readEntry("StartDragTime", 500);

    const QString buttonStyle = cg.readEntry("ToolButtonStyle", "TextBesideIcon").toLower();
    m_hints[ToolButtonStyle] = buttonStyle == "textbesideicon" ? Qt::ToolButtonTextBesideIcon
                             : buttonStyle == "icontextright" ? Qt::ToolButtonTextBesideIcon
                             : buttonStyle == "textundericon" ? Qt::ToolButtonTextUnderIcon
                             : buttonStyle == "icontextbottom" ? Qt::ToolButtonTextUnderIcon
                             : buttonStyle == "textonly" ? Qt::ToolButtonTextOnly
                             : Qt::ToolButtonIconOnly;

    m_hints[ToolBarIconSize] = cg.readEntry("ToolbarIconsSize", 22);
    m_hints[ItemViewActivateItemOnSingleClick] = cg.readEntry("SingleClick", true);
    m_hints[SystemIconThemeName] = cg.readEntry("IconsTheme", "oxygen");
    m_hints[SystemIconFallbackThemeName] = "hicolor";
    m_hints[IconThemeSearchPaths] = xdgIconThemePaths();
    m_hints[StyleNames] = (QStringList() << cg.readEntry("WidgetStyle", QString())
                                         << "oxygen"
                                         << "fusion"
                                         << "windows");
    m_hints[DialogButtonBoxLayout] = QDialogButtonBox::KdeLayout;
    m_hints[DialogButtonBoxButtonsHaveIcons] = cg.readEntry("ShowIconsOnPushButtons", true);
    m_hints[UseFullScreenForPopupMenu] = true;
    m_hints[KeyboardScheme] = KdeKeyboardScheme;
    m_hints[UiEffects] = cg.readEntry("GraphicEffectsLevel", 0) != 0 ? GeneralUiEffect : 0;
    m_hints[IconPixmapSizes] = QVariant::fromValue(QList<int>() << 512 << 256 << 128 << 64 << 32 << 22 << 16 << 8);
}

void KdePlatformTheme::loadPalettes()
{
    qDeleteAll(m_palettes);
    m_palettes.clear();

    KSharedConfig::Ptr globals = KSharedConfig::openConfig("kdeglobals");
    m_palettes[SystemPalette] = new QPalette(KColorScheme::createApplicationPalette(globals));
}

void KdePlatformTheme::setupIconLoader()
{
    connect(KIconLoader::global(), &KIconLoader::iconChanged, this, &KdePlatformTheme::iconChanged);
}

void KdePlatformTheme::iconChanged(int group)
{
    KIconLoader::Group iconGroup = (KIconLoader::Group) group;
    if (iconGroup != KIconLoader::Toolbar) {
        return;
    }

    const int currentSize = KIconLoader::global()->currentSize(KIconLoader::Toolbar);
    if (m_hints[ToolBarIconSize] == currentSize) {
        return;
    }

    m_hints[ToolBarIconSize] = currentSize;
    QWidgetList widgets = QApplication::allWidgets();
    Q_FOREACH(QWidget* widget, widgets) {
        if (qobject_cast<QToolBar*>(widget) || qobject_cast<QMainWindow*>(widget)) {
            QEvent event(QEvent::StyleChange);
            QApplication::sendEvent(widget, &event);
        }
    }
}
