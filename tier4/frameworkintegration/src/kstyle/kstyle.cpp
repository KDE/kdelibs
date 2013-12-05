/**
 * KStyle for KDE4
 * Copyright (C) 2004-2005 Maksim Orlovich <maksim@kde.org>
 * Copyright (C) 2005,2006 Sandro Giessl <giessl@kde.org>
 *
 * Based in part on the following software:
 *  KStyle for KDE3
 *      Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
 *      Portions  (C) 1998-2000 TrollTech AS
 *  Keramik for KDE3,
 *      Copyright (C) 2002      Malte Starostik   <malte@kde.org>
 *                (C) 2002-2003 Maksim Orlovich  <maksim@kde.org>
 *      Portions  (C) 2001-2002 Karol Szwed     <gallium@kde.org>
 *                (C) 2001-2002 Fredrik Höglund <fredrik@kde.org>
 *                (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *                (C) 2000 Dirk Mueller         <mueller@kde.org>
 *                (C) 2001 Martijn Klingens    <klingens@kde.org>
 *                (C) 2003 Sandro Giessl      <sandro@giessl.com>
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * Many thanks to Bradley T. Hughes for the 3 button scrollbar code.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kstyle.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QIcon>
#include <QStyleOption>
#include <QPushButton>
#include <QToolBar>

#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kcolorscheme.h>

KStyle::KStyle() : d(0)
{
}

KStyle::~KStyle()
{
}

void KStyle::polish(QWidget* w)
{
    // Enable hover effects in all itemviews
    if (QAbstractItemView *itemView = qobject_cast<QAbstractItemView*>(w)) {
        itemView->viewport()->setAttribute(Qt::WA_Hover);
    }

    if (QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(w)) {
        QPushButton* button = box->button(QDialogButtonBox::Ok);
        if (button && button->shortcut().isEmpty()) {
            button->setShortcut(Qt::CTRL | Qt::Key_Return);
        }
    }

    QCommonStyle::polish(w);
}

QPalette KStyle::standardPalette() const
{
    return KColorScheme::createApplicationPalette(KSharedConfig::openConfig());
}

QIcon KStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption */*option*/,
                                         const QWidget */*widget*/) const
{
    switch (standardIcon) {
        case QStyle::SP_DesktopIcon:
            return QIcon::fromTheme("user-desktop");
        case QStyle::SP_TrashIcon:
            return QIcon::fromTheme("user-trash");
        case QStyle::SP_ComputerIcon:
            return QIcon::fromTheme("computer");
        case QStyle::SP_DriveFDIcon:
            return QIcon::fromTheme("media-floppy");
        case QStyle::SP_DriveHDIcon:
            return QIcon::fromTheme("drive-harddisk");
        case QStyle::SP_DriveCDIcon:
        case QStyle::SP_DriveDVDIcon:
            return QIcon::fromTheme("drive-optical");
        case QStyle::SP_DriveNetIcon:
            return QIcon::fromTheme("folder-remote");
        case QStyle::SP_DirHomeIcon:
            return QIcon::fromTheme("user-home");
        case QStyle::SP_DirOpenIcon:
            return QIcon::fromTheme("document-open-folder");
        case QStyle::SP_DirClosedIcon:
            return QIcon::fromTheme("folder");
        case QStyle::SP_DirIcon:
            return QIcon::fromTheme("folder");
        case QStyle::SP_DirLinkIcon:
            return QIcon::fromTheme("folder"); //TODO: generate (!?) folder with link emblem
        case QStyle::SP_FileIcon:
            return QIcon::fromTheme("text-plain"); //TODO: look for a better icon
        case QStyle::SP_FileLinkIcon:
            return QIcon::fromTheme("text-plain"); //TODO: generate (!?) file with link emblem
        case QStyle::SP_FileDialogStart:
            return QIcon::fromTheme("media-playback-start"); //TODO: find correct icon
        case QStyle::SP_FileDialogEnd:
            return QIcon::fromTheme("media-playback-stop"); //TODO: find correct icon
        case QStyle::SP_FileDialogToParent:
            return QIcon::fromTheme("go-up");
        case QStyle::SP_FileDialogNewFolder:
            return QIcon::fromTheme("folder-new");
        case QStyle::SP_FileDialogDetailedView:
            return QIcon::fromTheme("view-list-details");
        case QStyle::SP_FileDialogInfoView:
            return QIcon::fromTheme("document-properties");
        case QStyle::SP_FileDialogContentsView:
            return QIcon::fromTheme("view-list-icons");
        case QStyle::SP_FileDialogListView:
            return QIcon::fromTheme("view-list-text");
        case QStyle::SP_FileDialogBack:
            return QIcon::fromTheme("go-previous");
        case QStyle::SP_MessageBoxInformation:
            return QIcon::fromTheme("dialog-information");
        case QStyle::SP_MessageBoxWarning:
            return QIcon::fromTheme("dialog-warning");
        case QStyle::SP_MessageBoxCritical:
            return QIcon::fromTheme("dialog-error");
        case QStyle::SP_MessageBoxQuestion:
            return QIcon::fromTheme("dialog-information");
        case QStyle::SP_DialogOkButton:
            return QIcon::fromTheme("dialog-ok");
        case QStyle::SP_DialogCancelButton:
            return QIcon::fromTheme("dialog-cancel");
        case QStyle::SP_DialogHelpButton:
            return QIcon::fromTheme("help-contents");
        case QStyle::SP_DialogOpenButton:
            return QIcon::fromTheme("document-open");
        case QStyle::SP_DialogSaveButton:
            return QIcon::fromTheme("document-save");
        case QStyle::SP_DialogCloseButton:
            return QIcon::fromTheme("dialog-close");
        case QStyle::SP_DialogApplyButton:
            return QIcon::fromTheme("dialog-ok-apply");
        case QStyle::SP_DialogResetButton:
            return QIcon::fromTheme("document-revert");
        case QStyle::SP_DialogDiscardButton:
            return QIcon::fromTheme("dialog-cancel");
        case QStyle::SP_DialogYesButton:
            return QIcon::fromTheme("dialog-ok-apply");
        case QStyle::SP_DialogNoButton:
            return QIcon::fromTheme("dialog-cancel");
        case QStyle::SP_ArrowUp:
            return QIcon::fromTheme("go-up");
        case QStyle::SP_ArrowDown:
            return QIcon::fromTheme("go-down");
        case QStyle::SP_ArrowLeft:
            return QIcon::fromTheme("go-previous-view");
        case QStyle::SP_ArrowRight:
            return QIcon::fromTheme("go-next-view");
        case QStyle::SP_ArrowBack:
            return QIcon::fromTheme("go-previous");
        case QStyle::SP_ArrowForward:
            return QIcon::fromTheme("go-next");
        case QStyle::SP_BrowserReload:
            return QIcon::fromTheme("view-refresh");
        case QStyle::SP_BrowserStop:
            return QIcon::fromTheme("process-stop");
        case QStyle::SP_MediaPlay:
            return QIcon::fromTheme("media-playback-start");
        case QStyle::SP_MediaStop:
            return QIcon::fromTheme("media-playback-stop");
        case QStyle::SP_MediaPause:
            return QIcon::fromTheme("media-playback-pause");
        case QStyle::SP_MediaSkipForward:
            return QIcon::fromTheme("media-skip-forward");
        case QStyle::SP_MediaSkipBackward:
            return QIcon::fromTheme("media-skip-backward");
        case QStyle::SP_MediaSeekForward:
            return QIcon::fromTheme("media-seek-forward");
        case QStyle::SP_MediaSeekBackward:
            return QIcon::fromTheme("media-seek-backward");
        case QStyle::SP_MediaVolume:
            return QIcon::fromTheme("audio-volume-medium");
        case QStyle::SP_MediaVolumeMuted:
            return QIcon::fromTheme("audio-volume-muted");

        default:
            return QIcon();
    }
}

int KStyle::styleHint (StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
    switch (hint)
    {
        case SH_ItemView_ActivateItemOnSingleClick:
        {
            KConfigGroup g(KSharedConfig::openConfig(), "KDE");
            return g.readEntry("SingleClick", true );
        }

        case SH_DialogButtonBox_ButtonsHaveIcons:
        {
            // was KGlobalSettings::showIconsOnPushButtons() :
            KConfigGroup g(KSharedConfig::openConfig(), "KDE");
            return g.readEntry("ShowIconsOnPushButtons", true);
        }

        case SH_ItemView_ArrowKeysNavigateIntoChildren:
            return true;

        case SH_Widget_Animate:
        {
            KConfigGroup g(KSharedConfig::openConfig(), "KDE-Global GUI Settings");
            return g.readEntry("GraphicEffectsLevel", 0);
        }

        case SH_ToolButtonStyle:
        {
            KConfigGroup g(KSharedConfig::openConfig(), "KDE");

            bool useOthertoolbars = false;
            const QWidget *parent = widget->parentWidget();

            //If the widget parent is a QToolBar and the magic property is set
            if (parent && qobject_cast< const QToolBar* >(parent)) {
                if (parent->property("otherToolbar").isValid()) {
                    useOthertoolbars = true;
                }
            }

            QString buttonStyle;
            if (useOthertoolbars) {
                buttonStyle = g.readEntry("ToolButtonStyleOtherToolbars", "NoText").toLower();
            } else {
                buttonStyle = g.readEntry("ToolButtonStyle", "TextBesideIcon").toLower();
            }

            return buttonStyle == QLatin1String("textbesideicon") ? Qt::ToolButtonTextBesideIcon
                             : buttonStyle == QLatin1String("icontextright") ? Qt::ToolButtonTextBesideIcon
                             : buttonStyle == QLatin1String("textundericon") ? Qt::ToolButtonTextUnderIcon
                             : buttonStyle == QLatin1String("icontextbottom") ? Qt::ToolButtonTextUnderIcon
                             : buttonStyle == QLatin1String("textonly") ? Qt::ToolButtonTextOnly
                             : Qt::ToolButtonIconOnly;
        }
        default:
            break;
    };

    return QCommonStyle::styleHint(hint, option, widget, returnData);
}

int KStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    switch (metric)
    {
        case PM_SmallIconSize:
        case PM_ButtonIconSize:
            return KIconLoader::global()->currentSize(KIconLoader::Small);

        case PM_ToolBarIconSize:
            return KIconLoader::global()->currentSize(KIconLoader::Toolbar);

        case PM_LargeIconSize:
            return KIconLoader::global()->currentSize(KIconLoader::Dialog);

        case PM_MessageBoxIconSize:
            // TODO return KIconLoader::global()->currentSize(KIconLoader::MessageBox);
            return KIconLoader::SizeHuge;
        default:
            break;
    }

    return QCommonStyle::pixelMetric(metric, option, widget);
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
