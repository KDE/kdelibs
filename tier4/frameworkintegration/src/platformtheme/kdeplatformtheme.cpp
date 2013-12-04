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
#include "kdeplatformfiledialoghelper.h"

#include <QCoreApplication>
#include <QFont>
#include <QPalette>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <kiconengine.h>
#include <kiconloader.h>
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
        case QKeySequence::WhatsThis:
            return KStandardShortcut::shortcut(KStandardShortcut::WhatsThis);
        case QKeySequence::Open:
            return KStandardShortcut::shortcut(KStandardShortcut::Open);
        case QKeySequence::Close:
            return KStandardShortcut::shortcut(KStandardShortcut::Close);
        case QKeySequence::Save:
            return KStandardShortcut::shortcut(KStandardShortcut::Save);
        case QKeySequence::New:
            return KStandardShortcut::shortcut(KStandardShortcut::New);
        case QKeySequence::Cut:
            return KStandardShortcut::shortcut(KStandardShortcut::Cut);
        case QKeySequence::Copy:
            return KStandardShortcut::shortcut(KStandardShortcut::Copy);
        case QKeySequence::Paste:
            return KStandardShortcut::shortcut(KStandardShortcut::Paste);
        case QKeySequence::Undo:
            return KStandardShortcut::shortcut(KStandardShortcut::Undo);
        case QKeySequence::Redo:
            return KStandardShortcut::shortcut(KStandardShortcut::Redo);
        case QKeySequence::Back:
            return KStandardShortcut::shortcut(KStandardShortcut::Back);
        case QKeySequence::Forward:
            return KStandardShortcut::shortcut(KStandardShortcut::Forward);
        case QKeySequence::Refresh:
            return KStandardShortcut::shortcut(KStandardShortcut::Reload);
        case QKeySequence::ZoomIn:
            return KStandardShortcut::shortcut(KStandardShortcut::ZoomIn);
        case QKeySequence::ZoomOut:
            return KStandardShortcut::shortcut(KStandardShortcut::ZoomOut);
        case QKeySequence::Print:
            return KStandardShortcut::shortcut(KStandardShortcut::Print);
        case QKeySequence::Find:
            return KStandardShortcut::shortcut(KStandardShortcut::Find);
        case QKeySequence::FindNext:
            return KStandardShortcut::shortcut(KStandardShortcut::FindNext);
        case QKeySequence::FindPrevious:
            return KStandardShortcut::shortcut(KStandardShortcut::FindPrev);
        case QKeySequence::Replace:
            return KStandardShortcut::shortcut(KStandardShortcut::Replace);
        case QKeySequence::SelectAll:
            return KStandardShortcut::shortcut(KStandardShortcut::SelectAll);
        case QKeySequence::MoveToNextWord:
            return KStandardShortcut::shortcut(KStandardShortcut::ForwardWord);
        case QKeySequence::MoveToPreviousWord:
            return KStandardShortcut::shortcut(KStandardShortcut::BackwardWord);
        case QKeySequence::MoveToNextPage:
            return KStandardShortcut::shortcut(KStandardShortcut::Next);
        case QKeySequence::MoveToPreviousPage:
            return KStandardShortcut::shortcut(KStandardShortcut::Prior);
        case QKeySequence::MoveToStartOfLine:
            return KStandardShortcut::shortcut(KStandardShortcut::BeginningOfLine);
        case QKeySequence::MoveToEndOfLine:
            return KStandardShortcut::shortcut(KStandardShortcut::EndOfLine);
        case QKeySequence::MoveToStartOfDocument:
            return KStandardShortcut::shortcut(KStandardShortcut::Home);
        case QKeySequence::MoveToEndOfDocument:
            return KStandardShortcut::shortcut(KStandardShortcut::End);
        case QKeySequence::SaveAs:
            return KStandardShortcut::shortcut(KStandardShortcut::SaveAs);
        case QKeySequence::Preferences:
            return KStandardShortcut::shortcut(KStandardShortcut::Preferences);
        case QKeySequence::Quit:
            return KStandardShortcut::shortcut(KStandardShortcut::Quit);
        case QKeySequence::FullScreen:
            return KStandardShortcut::shortcut(KStandardShortcut::FullScreen);
        case QKeySequence::Deselect:
            return KStandardShortcut::shortcut(KStandardShortcut::Deselect);
        default:
            return QPlatformTheme::keyBindings(key);
    }
}

bool KdePlatformTheme::usePlatformNativeDialog(QPlatformTheme::DialogType type) const
{
    return type == QPlatformTheme::FileDialog;
}

QPlatformDialogHelper* KdePlatformTheme::createPlatformDialogHelper(QPlatformTheme::DialogType type) const
{
    switch(type) {
        case QPlatformTheme::FileDialog:
            return new KDEPlatformFileDialogHelper;
        case QPlatformTheme::FontDialog:
        case QPlatformTheme::ColorDialog:
        case QPlatformTheme::MessageDialog:
        default:
            return 0;
    }
}
