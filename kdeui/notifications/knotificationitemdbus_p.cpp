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

#include "knotificationitemdbus_p.h"
#include "knotificationitemprivate_p.h"
#include "knotificationitem.h"

#include <QDBusConnection>
#include <QPixmap>
#include <QImage>
#include <QApplication>
#include <QMovie>


#include <kdebug.h>
#include <ksystemtrayicon.h>
#include <kaboutdata.h>
#include <kicon.h>
#include <kmenu.h>
#include <kaction.h>
#include <kwindowinfo.h>
#include <kwindowsystem.h>
#include <kmessagebox.h>
#include <kactioncollection.h>

#include "statusnotifierwatcher_interface.h"


#include "statusnotifieritemadaptor.h"

// Marshall the ImageStruct data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const KDbusImageStruct &icon)
{
    argument.beginStructure();
    argument << icon.width;
    argument << icon.height;
    argument << icon.data;
    argument.endStructure();
    return argument;
}

// Retrieve the ImageStruct data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusImageStruct &icon)
{
    qint32 width;
    qint32 height;
    QByteArray data;

    argument.beginStructure();
    argument >> width;
    argument >> height;
    argument >> data;
    argument.endStructure();

    icon.width = width;
    icon.height = height;
    icon.data = data;

    return argument;
}


// Marshall the ImageVector data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const KDbusImageVector &iconVector)
{
    argument.beginArray(qMetaTypeId<KDbusImageStruct>());
    for (int i=0; i<iconVector.size(); ++i) {
        argument << iconVector[i]; 
    }
    argument.endArray();
    return argument;
}

// Retrieve the ImageVector data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusImageVector &iconVector)
{
    argument.beginArray();
    iconVector.clear();

    while ( !argument.atEnd() ) {
       KDbusImageStruct element;
       argument >> element;
       iconVector.append(element);
    }

    argument.endArray();


    return argument;
}

// Marshall the ToolTipStruct data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const KDbusToolTipStruct &toolTip)
{
    argument.beginStructure();
    argument << toolTip.icon;
    argument << toolTip.image;
    argument << toolTip.title;
    argument << toolTip.subTitle;
    argument.endStructure();
    return argument;
}

// Retrieve the ToolTipStruct data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, KDbusToolTipStruct &toolTip)
{
    QString icon;
    KDbusImageVector image;
    QString title;
    QString subTitle;

    argument.beginStructure();
    argument >> icon;
    argument >> image;
    argument >> title;
    argument >> subTitle;
    argument.endStructure();

    toolTip.icon = icon;
    toolTip.image = image;
    toolTip.title = title;
    toolTip.subTitle = subTitle;

    return argument;
}


int KNotificationItemDBus::s_serviceCount = 0;

KNotificationItemDBus::KNotificationItemDBus(KNotificationItem *parent)
  : QObject(parent),
    m_notificationItem(parent),
    m_service(QString("org.kde.NotificationItem-%1-%2")
                      .arg(QCoreApplication::applicationPid())
                      .arg(++s_serviceCount)),
    m_dbus(QDBusConnection::connectToBus(QDBusConnection::SessionBus, m_service))
{
   new StatusNotifierItemAdaptor(this);
   kDebug(299) << "service is" << m_service;
   m_dbus.registerService(m_service);
   m_dbus.registerObject("/NotificationItem", this);
}

KNotificationItemDBus::~KNotificationItemDBus()
{
    m_dbus.unregisterService(m_service);
}

QString KNotificationItemDBus::service() const
{
    return m_service;
}

//DBUS slots

QString KNotificationItemDBus::Category() const
{
    return m_notificationItem->metaObject()->enumerator(m_notificationItem->metaObject()->indexOfEnumerator("ItemCategory")).valueToKey(m_notificationItem->category());
}

QString KNotificationItemDBus::Title() const
{
    return m_notificationItem->title();
}

QString KNotificationItemDBus::Id() const
{
    return m_notificationItem->id();
}

QString KNotificationItemDBus::Status() const
 {
    return m_notificationItem->metaObject()->enumerator(m_notificationItem->metaObject()->indexOfEnumerator("ItemStatus")).valueToKey(m_notificationItem->status());
}

int KNotificationItemDBus::WindowId() const
{
    if (m_notificationItem->d->associatedWidget) {
        return (int)m_notificationItem->d->associatedWidget->winId();
    } else {
        return 0;
    }
}


//Icon

QString KNotificationItemDBus::IconName() const
{
    return m_notificationItem->iconName();
}

KDbusImageVector KNotificationItemDBus::IconPixmap() const
{
    return m_notificationItem->d->serializedIcon;
}

QString KNotificationItemDBus::OverlayIconName() const
{
    return m_notificationItem->overlayIconName();
}

KDbusImageVector KNotificationItemDBus::OverlayIconPixmap() const
{
    return m_notificationItem->d->serializedOverlayIcon;
}

//Requesting attention icon and movie

QString KNotificationItemDBus::AttentionIconName() const
{
    return m_notificationItem->attentionIconName();
}

KDbusImageVector KNotificationItemDBus::AttentionIconPixmap() const
{
    return m_notificationItem->d->serializedAttentionIcon;
}

KDbusImageVector KNotificationItemDBus::AttentionMovie() const
{
    return m_notificationItem->d->movieVector;
}


//ToolTip

KDbusToolTipStruct KNotificationItemDBus::ToolTip() const
{
    KDbusToolTipStruct toolTip;
    toolTip.icon = m_notificationItem->toolTipIconName();
    toolTip.image = m_notificationItem->d->serializedToolTipIcon;
    toolTip.title = m_notificationItem->toolTipTitle();
    toolTip.subTitle = m_notificationItem->toolTipSubTitle();

    return toolTip;
}

//Interaction

void KNotificationItemDBus::ContextMenu(int x, int y)
{
    if (!m_notificationItem->d->menu) {
        return;
    }

    //TODO: nicer placement, possible?
    if (!m_notificationItem->d->menu->isVisible()) {
        m_notificationItem->d->menu->setWindowFlags(Qt::Window|Qt::FramelessWindowHint);
        m_notificationItem->d->menu->popup(QPoint(x,y));
        KWindowSystem::setState(m_notificationItem->d->menu->winId(), NET::SkipTaskbar|NET::SkipPager|NET::KeepAbove);
        KWindowSystem::setType(m_notificationItem->d->menu->winId(), NET::PopupMenu);
        KWindowSystem::forceActiveWindow(m_notificationItem->d->menu->winId());
    } else {
        m_notificationItem->d->menu->hide();
    }
}

void KNotificationItemDBus::Activate(int x, int y)
{
    if (m_notificationItem->d->associatedWidget == m_notificationItem->d->menu) {
        ContextMenu(x, y);
    } else {
        m_notificationItem->activate(QPoint(x,y));
    }
}

void KNotificationItemDBus::SecondaryActivate(int x, int y)
{
    emit m_notificationItem->secondaryActivateRequested(QPoint(x,y));
}

void KNotificationItemDBus::Scroll(int delta, const QString &orientation)
{
    Qt::Orientation dir = (orientation.toLower() == "horizontal" ? Qt::Horizontal : Qt::Vertical);
    emit m_notificationItem->scrollRequested(delta, dir);
}



#include "knotificationitemdbus_p.moc"
