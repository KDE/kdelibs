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
#include "khintssettings.h"

#include <QCoreApplication>
#include <QFont>
#include <QPalette>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <kcolorscheme.h>
#include <kconfiggroup.h>
#include <kiconengine_p.h>
#include <kiconloader.h>
#include <ksharedconfig.h>
#include <kstandardshortcut.h>

KdePlatformTheme::KdePlatformTheme()
{
    loadSettings();
}

KdePlatformTheme::~KdePlatformTheme()
{
    delete m_fontsData;
    delete m_hints;
}

QVariant KdePlatformTheme::themeHint(QPlatformTheme::ThemeHint hintType) const
{
    QVariant hint = m_hints->hint(hintType);
    if (hint.isValid()) {
        return hint;
    } else {
        return QPlatformTheme::themeHint(hintType);
    }
}

const QPalette *KdePlatformTheme::palette(Palette type) const
{
    QPalette *palette = m_hints->palette(type);
    if (palette)
        return palette;
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

void KdePlatformTheme::loadSettings()
{
    m_fontsData = new KFontSettingsData;
    m_hints = new KHintsSettings;
}


QList<QKeySequence> KdePlatformTheme::keyBindings(QKeySequence::StandardKey key) const
{
    switch (key) {
        case QKeySequence::HelpContents:
            return KStandardShortcut::shortcut(KStandardShortcut::Help);
            break;
        case QKeySequence::WhatsThis:
            return KStandardShortcut::shortcut(KStandardShortcut::WhatsThis);
            break;
        case QKeySequence::Open:
            return KStandardShortcut::shortcut(KStandardShortcut::Open);
            break;
        case QKeySequence::Close:
            return KStandardShortcut::shortcut(KStandardShortcut::Close);
            break;
        case QKeySequence::Save:
            return KStandardShortcut::shortcut(KStandardShortcut::Save);
            break;
        case QKeySequence::New:
            return KStandardShortcut::shortcut(KStandardShortcut::New);
            break;
        case QKeySequence::Cut:
            return KStandardShortcut::shortcut(KStandardShortcut::Cut);
            break;
        case QKeySequence::Copy:
            return KStandardShortcut::shortcut(KStandardShortcut::Copy);
            break;
        case QKeySequence::Paste:
            return KStandardShortcut::shortcut(KStandardShortcut::Paste);
            break;
        case QKeySequence::Undo:
            return KStandardShortcut::shortcut(KStandardShortcut::Undo);
            break;
        case QKeySequence::Redo:
            return KStandardShortcut::shortcut(KStandardShortcut::Redo);
            break;
        case QKeySequence::Back:
            return KStandardShortcut::shortcut(KStandardShortcut::Back);
            break;
        case QKeySequence::Forward:
            return KStandardShortcut::shortcut(KStandardShortcut::Forward);
            break;
        case QKeySequence::Refresh:
            return KStandardShortcut::shortcut(KStandardShortcut::Reload);
            break;
        case QKeySequence::ZoomIn:
            return KStandardShortcut::shortcut(KStandardShortcut::ZoomIn);
            break;
        case QKeySequence::ZoomOut:
            return KStandardShortcut::shortcut(KStandardShortcut::ZoomOut);
            break;
        case QKeySequence::Print:
            return KStandardShortcut::shortcut(KStandardShortcut::Print);
            break;
        case QKeySequence::Find:
            return KStandardShortcut::shortcut(KStandardShortcut::Find);
            break;
        case QKeySequence::FindNext:
            return KStandardShortcut::shortcut(KStandardShortcut::FindNext);
            break;
        case QKeySequence::FindPrevious:
            return KStandardShortcut::shortcut(KStandardShortcut::FindPrev);
            break;
        case QKeySequence::Replace:
            return KStandardShortcut::shortcut(KStandardShortcut::Replace);
            break;
        case QKeySequence::SelectAll:
            return KStandardShortcut::shortcut(KStandardShortcut::SelectAll);
            break;
        case QKeySequence::MoveToNextWord:
            return KStandardShortcut::shortcut(KStandardShortcut::ForwardWord);
            break;
        case QKeySequence::MoveToPreviousWord:
            return KStandardShortcut::shortcut(KStandardShortcut::BackwardWord);
            break;
        case QKeySequence::MoveToNextPage:
            return KStandardShortcut::shortcut(KStandardShortcut::Next);
            break;
        case QKeySequence::MoveToPreviousPage:
            return KStandardShortcut::shortcut(KStandardShortcut::Prior);
            break;
        case QKeySequence::MoveToStartOfLine:
            return KStandardShortcut::shortcut(KStandardShortcut::BeginningOfLine);
            break;
        case QKeySequence::MoveToEndOfLine:
            return KStandardShortcut::shortcut(KStandardShortcut::EndOfLine);
            break;
        case QKeySequence::MoveToStartOfDocument:
            return KStandardShortcut::shortcut(KStandardShortcut::Home);
            break;
        case QKeySequence::MoveToEndOfDocument:
            return KStandardShortcut::shortcut(KStandardShortcut::End);
            break;
        case QKeySequence::SaveAs:
            return KStandardShortcut::shortcut(KStandardShortcut::SaveAs);
            break;
        case QKeySequence::Preferences:
            return KStandardShortcut::shortcut(KStandardShortcut::Preferences);
            break;
        case QKeySequence::Quit:
            return KStandardShortcut::shortcut(KStandardShortcut::Quit);
            break;
        case QKeySequence::FullScreen:
            return KStandardShortcut::shortcut(KStandardShortcut::FullScreen);
            break;
        case QKeySequence::Deselect:
            return KStandardShortcut::shortcut(KStandardShortcut::Deselect);
            break;
        default:
            return QPlatformTheme::keyBindings(key);
    }
}
