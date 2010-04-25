/* This file is part of the KDE libraries
   Copyright 2009 by Marco Martin <notmart@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KSTATUSNOTIFIERITEMPRIVATE_H
#define KSTATUSNOTIFIERITEMPRIVATE_H

#include <QObject>
#include <QString>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QPixmap>
#include <QWheelEvent>

#include <ksystemtrayicon.h>

#include "kstatusnotifieritem.h"
#include "kstatusnotifieritemdbus_p.h"

#include "statusnotifierwatcher_interface.h"
#include "notifications_interface.h"

class KActionCollection;
class KSystemTrayIcon;
class KMenu;
class QMovie;
class QAction;


// this class is needed because we can't just put an event filter on it:
// the events that are passed to QSystemTrayIcon are done so in a way that
// bypasses the usual event filtering mechanisms *sigh*
class KStatusNotifierLegacyIcon : public KSystemTrayIcon
{
    Q_OBJECT

public:
    KStatusNotifierLegacyIcon(QWidget *parent)
        : KSystemTrayIcon(parent)
    {
    }

    bool event(QEvent *e)
    {
        if (e->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(e);
            emit wheel(wheelEvent->delta());
        }

        return false;
    }

signals:
    void wheel(int);
};

class KStatusNotifierItemPrivate
{
public:
    KStatusNotifierItemPrivate(KStatusNotifierItem *item);

    void init(const QString &extraId);
    void registerToDaemon();
    void serviceChange(const QString &name, const QString &oldOwner, const QString &newOwner);
    void setLegacySystemTrayEnabled(bool enabled);
    void syncLegacySystemTrayIcon();
    void contextMenuAboutToShow();
    void maybeQuit();
    void minimizeRestore();
    void minimizeRestore(bool show);
    void hideMenu();
    void setLegacyMode(bool legacy);
    void checkForRegisteredHosts();
    void legacyWheelEvent(int delta);
    void legacyActivated(QSystemTrayIcon::ActivationReason reason);

    KDbusImageStruct imageToStruct(const QImage &image);
    KDbusImageVector iconToVector(const QIcon &icon);
    bool checkVisibility(QPoint pos, bool perform = true);

    static const int s_protocolVersion;

    KStatusNotifierItem *q;

    KStatusNotifierItem::ItemCategory category;
    QString id;
    QString title;
    KStatusNotifierItem::ItemStatus status;

    QString iconName;
    KDbusImageVector serializedIcon;
    QIcon icon;

    QString overlayIconName;
    KDbusImageVector serializedOverlayIcon;
    QIcon overlayIcon;

    QString attentionIconName;
    QIcon attentionIcon;
    KDbusImageVector serializedAttentionIcon;
    QString movieName;
    QPointer<QMovie> movie;

    QString toolTipIconName;
    KDbusImageVector serializedToolTipIcon;
    QIcon toolTipIcon;
    QString toolTipTitle;
    QString toolTipSubTitle;
    QString iconThemePath;
    QString menuObjectPath;

    KMenu *menu;
    KActionCollection* actionCollection;
    QWidget *associatedWidget;
    QAction* titleAction;
    org::kde::StatusNotifierWatcher *statusNotifierWatcher;
    org::freedesktop::Notifications *notificationsClient;

    KSystemTrayIcon *systemTrayIcon;
    KStatusNotifierItemDBus *statusNotifierItemDBus;

    bool hasQuit : 1;
    bool onAllDesktops : 1;
    bool standardActionsEnabled : 1;
};

#endif
