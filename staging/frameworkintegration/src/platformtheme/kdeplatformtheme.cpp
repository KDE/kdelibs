/*  This file is part of the KDE libraries
 *  Copyright 2013 Kevin Ottens <ervin+bluesystems@kde.org>
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
    qDeleteAll(m_fonts);
    qDeleteAll(m_palettes);
}

QVariant KdePlatformTheme::themeHint(QPlatformTheme::ThemeHint hint) const
{
    if (m_hints.contains(hint))
        return m_hints.value(hint);
    else
        return QPlatformTheme::themeHint(hint);
}

const QPalette *KdePlatformTheme::palette(Palette type) const
{
    if (m_palettes.contains(type))
        return m_palettes.value(type);
    else
        return QPlatformTheme::palette(type);
}

const QFont *KdePlatformTheme::font(Font type) const
{
    if (m_fonts.contains(type))
        return m_fonts.value(type);
    else
        return QPlatformTheme::font(type);
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
    loadFonts();
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

    QPalette palette;

    QList<QPalette::ColorGroup> groups;
    groups << QPalette::Disabled << QPalette::Active << QPalette::Inactive;
    foreach (QPalette::ColorGroup group, groups) {
        KColorScheme windowScheme(group, KColorScheme::Window, KSharedConfig::openConfig("kdeglobals"));
        palette.setBrush(group, QPalette::Window, windowScheme.background(KColorScheme::NormalBackground));
        palette.setBrush(group, QPalette::WindowText, windowScheme.foreground(KColorScheme::NormalText));
        palette.setBrush(group, QPalette::BrightText, windowScheme.foreground(KColorScheme::NegativeText));
        palette.setBrush(group, QPalette::Link, windowScheme.foreground(KColorScheme::LinkText));
        palette.setBrush(group, QPalette::LinkVisited, windowScheme.foreground(KColorScheme::VisitedText));

        palette.setBrush(group, QPalette::Light, windowScheme.shade(KColorScheme::LightShade));
        palette.setBrush(group, QPalette::Midlight, windowScheme.shade(KColorScheme::MidlightShade));
        palette.setBrush(group, QPalette::Dark, windowScheme.shade(KColorScheme::DarkShade));
        palette.setBrush(group, QPalette::Mid, windowScheme.shade(KColorScheme::MidShade));
        palette.setBrush(group, QPalette::Shadow, windowScheme.shade(KColorScheme::ShadowShade));

        KColorScheme viewScheme(group, KColorScheme::View, KSharedConfig::openConfig("kdeglobals"));
        palette.setBrush(group, QPalette::Base, viewScheme.background(KColorScheme::NormalBackground));
        palette.setBrush(group, QPalette::AlternateBase, viewScheme.background(KColorScheme::AlternateBackground));
        palette.setBrush(group, QPalette::Text, viewScheme.foreground(KColorScheme::NormalText));

        KColorScheme tooltipScheme(group, KColorScheme::Tooltip, KSharedConfig::openConfig("kdeglobals"));
        palette.setBrush(group, QPalette::ToolTipBase, tooltipScheme.background(KColorScheme::NormalBackground));
        palette.setBrush(group, QPalette::ToolTipText, tooltipScheme.foreground(KColorScheme::NormalText));

        KColorScheme buttonScheme(group, KColorScheme::Button, KSharedConfig::openConfig("kdeglobals"));
        palette.setBrush(group, QPalette::Button, buttonScheme.background(KColorScheme::NormalBackground));
        palette.setBrush(group, QPalette::ButtonText, buttonScheme.foreground(KColorScheme::NormalText));

        KColorScheme selectionScheme(group, KColorScheme::Selection, KSharedConfig::openConfig("kdeglobals"));
        palette.setBrush(group, QPalette::Highlight, selectionScheme.background(KColorScheme::NormalBackground));
        palette.setBrush(group, QPalette::HighlightedText, selectionScheme.foreground(KColorScheme::NormalText));
    }

    m_palettes[SystemPalette] = new QPalette(palette);
}

void KdePlatformTheme::loadFonts()
{
    qDeleteAll(m_fonts);
    m_fonts.clear();

    KConfigGroup cg(KSharedConfig::openConfig("kdeglobals"), "General");
    QFont defaultFont = QFont("Sans Serif", 9);
    defaultFont.setStyleHint(QFont::SansSerif);
    QFont defaultFixedFont = QFont("Monospace", 9);

    const QFont generalFont = cg.readEntry("font", defaultFont);
    const QFont fixedFont = cg.readEntry("fixed", defaultFixedFont);
    const QFont menuFont = cg.readEntry("menuFont", defaultFont);

    defaultFont.setPointSize(8);
    const QFont toolBarFont = cg.readEntry("toolBarFont", defaultFont);
    const QFont smallestReadableFont = cg.readEntry("smallestReadableFont", defaultFont);

    KConfigGroup wmcg(KSharedConfig::openConfig("kdeglobals"), "WM");
    const QFont windowTitleFont = wmcg.readEntry("activeFont", defaultFont);


    QCoreApplication::instance()->setProperty("_k_fixedFont", QVariant::fromValue(fixedFont));

    m_fonts[SystemFont] = new QFont(generalFont);
    m_fonts[MenuFont] = new QFont(menuFont);
    m_fonts[MenuBarFont] = new QFont(menuFont);
    m_fonts[MenuItemFont] = new QFont(menuFont);
    m_fonts[MessageBoxFont] = new QFont(generalFont);
    m_fonts[LabelFont] = new QFont(generalFont);
    m_fonts[TipLabelFont] = new QFont(generalFont);
    m_fonts[StatusBarFont] = new QFont(generalFont);
    m_fonts[TitleBarFont] = new QFont(windowTitleFont);
    m_fonts[MdiSubWindowTitleFont] = new QFont(windowTitleFont);
    m_fonts[DockWidgetTitleFont] = new QFont(smallestReadableFont);
    m_fonts[PushButtonFont] = new QFont(generalFont);
    m_fonts[ToolButtonFont] = new QFont(toolBarFont);
    m_fonts[ItemViewFont] = new QFont(generalFont);
    m_fonts[ListViewFont] = new QFont(generalFont);
    m_fonts[HeaderViewFont] = new QFont(generalFont);
    m_fonts[ListBoxFont] = new QFont(generalFont);
    m_fonts[ComboMenuItemFont] = new QFont(generalFont);
    m_fonts[ComboLineEditFont] = new QFont(generalFont);
    m_fonts[SmallFont] = new QFont(smallestReadableFont);
    m_fonts[MiniFont] = new QFont(smallestReadableFont);
}

