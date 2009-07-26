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

#ifndef KNOTIFICATIONITEMPRIVATE_H
#define KNOTIFICATIONITEMPRIVATE_H

#include <QObject>
#include <QString>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QPixmap>
#include <QWheelEvent>

#include <ksystemtrayicon.h>

#include "knotificationitem.h"
#include "knotificationitemdbus_p.h"

#include "notificationwatcher_interface.h"
#include "notifications_interface.h"

class KActionCollection;
class KSystemTrayIcon;
class KMenu;
class QMovie;
class QAction;


namespace Experimental
{

// this class is needed because we can't just put an event filter on it:
// the events that are passed to QSystemTrayIcon are done so in a way that
// bypasses the usual event filtering mechanisms *sigh*
class KNotificationLegacyIcon : public KSystemTrayIcon
{
    Q_OBJECT

public:
    KNotificationLegacyIcon(QWidget *parent)
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

class KNotificationItemPrivate
{
public:
    KNotificationItemPrivate(KNotificationItem *item);

    void init(const QString &extraId);
    void registerToDaemon();
    void serviceChange(const QString& name, const QString& oldOwner, const QString& newOwner);
    void setLegacySystemTrayEnabled(bool enabled);
    void syncLegacySystemTrayIcon();
    void contextMenuAboutToShow();
    void maybeQuit();
    void minimizeRestore();
    void minimizeRestore(bool show);
    void hideMenu();
    void legacyWheelEvent(int delta);
    void legacyActivated(QSystemTrayIcon::ActivationReason reason);

    ExperimentalKDbusImageStruct imageToStruct(const QImage &image);
    ExperimentalKDbusImageVector iconToVector(const QIcon &icon);
    bool checkVisibility(QPoint pos, bool perform = true);

    static const int s_protocolVersion;

    KNotificationItem *q;

    QDBusConnection dbus;
    KNotificationItem::ItemCategory category;
    QString id;
    QString title;
    KNotificationItem::ItemStatus status;

    QString iconName;
    ExperimentalKDbusImageVector serializedIcon;
    QIcon icon;

    QString overlayIconName;
    ExperimentalKDbusImageVector serializedOverlayIcon;
    QIcon overlayIcon;

    QString attentionIconName;
    QIcon attentionIcon;
    ExperimentalKDbusImageVector serializedAttentionIcon;
    QMovie *movie;
    ExperimentalKDbusImageVector movieVector;

    QString toolTipIconName;
    ExperimentalKDbusImageVector serializedToolTipIcon;
    QIcon toolTipIcon;
    QString toolTipTitle;
    QString toolTipSubTitle;

    KMenu *menu;
    KActionCollection* actionCollection;
    QWidget *associatedWidget;
    QAction* titleAction;
    org::kde::NotificationItemWatcher *notificationItemWatcher;
    org::freedesktop::Notifications *notificationsClient;

    KSystemTrayIcon *systemTrayIcon;
    KNotificationItemDBus *notificationItemDbus;

    bool hasQuit : 1;
    bool onAllDesktops : 1;
    bool standardActionsEnabled : 1;
};

} // namespace Experimental
#endif
