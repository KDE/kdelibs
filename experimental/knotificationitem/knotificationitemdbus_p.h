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

#ifndef KNOTIFICATIONITEMDBUS_H
#define KNOTIFICATIONITEMDBUS_H

#include <kdedmodule.h>

#include <QObject>
#include <QString>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QPixmap>

//Custom message type for DBus
struct ExperimentalKDbusImageStruct {
    int width;
    int height;
    QByteArray data;
};

typedef QVector<ExperimentalKDbusImageStruct> ExperimentalKDbusImageVector;

struct ExperimentalKDbusToolTipStruct {
    QString icon;
    ExperimentalKDbusImageVector image;
    QString title;
    QString subTitle;
};

namespace Experimental
{

class KNotificationItem;

class KNotificationItemDBus : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString Category READ Category)
    Q_PROPERTY(QString Title READ Title)
    Q_PROPERTY(QString Status READ Status)
    Q_PROPERTY(int WindowId READ WindowId)
    Q_PROPERTY(QString Icon READ Icon)
    Q_PROPERTY(ExperimentalKDbusImageVector Image READ Image)
    Q_PROPERTY(QString OverlayIcon READ OverlayIcon)
    Q_PROPERTY(ExperimentalKDbusImageVector OverlayImage READ OverlayImage)
    Q_PROPERTY(QString AttentionIcon READ AttentionIcon)
    Q_PROPERTY(ExperimentalKDbusImageVector AttentionImage READ AttentionImage)
    Q_PROPERTY(ExperimentalKDbusImageVector AttentionMovie READ AttentionMovie)
    Q_PROPERTY(ExperimentalKDbusToolTipStruct ToolTip READ ToolTip)

    friend class KNotificationItem;
public:
    KNotificationItemDBus(KNotificationItem *parent);
    ~KNotificationItemDBus();

    /**
     * @return the service this object is registered on the bus under
     */
    QString service() const;

    /**
     * @return the category of the application associated to this icon
     * @see Category
     */
    QString Category() const;

    //FIXME:Immutable or a newTitle signal needed?
    /**
     * @return the title of this icon
     */
    QString Title() const;

    /**
     * @return The status of this icon
     * @see Status
     */
    QString Status() const;

    /**
     * @return The id of the main window of the application that controls the icon
     */
    int WindowId() const;

    /**
     * @return the name of the main icon to be displayed
     * if image() is not empty this will always return an empty string
     */
    QString Icon() const;

    /**
     * @return a serialization of the icon data
     */
    ExperimentalKDbusImageVector Image() const;

    /**
     * @return the name of the overlay of the main icon to be displayed
     * if image() is not empty this will always return an empty string
     */
    QString OverlayIcon() const;

    /**
     * @return a serialization of the icon data
     */
    ExperimentalKDbusImageVector OverlayImage() const;

    /**
     * @return the name of the icon to be displayed when the application
     * is requesting the user's attention
     * if attentionImage() is not empty this will always return an empty string
     */
    QString AttentionIcon() const;

    /**
     * @return a serialization of the requesting attention icon data
     */
    ExperimentalKDbusImageVector AttentionImage() const;

    /**
     * @return a serialization of the requesting attention movie data
     */
    ExperimentalKDbusImageVector AttentionMovie() const;

    /**
     * all the data needed for a tooltip
     */
    ExperimentalKDbusToolTipStruct ToolTip() const;


public Q_SLOTS:
    //interaction
    /**
     * Shows the context menu associated to this icon
     * at the desidered screen position
     */
    void ContextMenu(int x, int y);

    /**
     * Shows the main widget and try to position it on top
     * of the other windows, if the widget is already visible, hide it.
     */
    void Activate(int x, int y);

    /**
     * The user activated the item in an alternate way (for instance with middle mouse button, this depends from the systray implementation)
     */
    void SecondaryActivate(int x, int y);

    //FIXME: should relly be killed this one?
    /**
     * Inform this icon that the mouse wheel was used on its representation
     */
    void Scroll(int delta, const QString &orientation);

Q_SIGNALS:
    /**
     * Inform the systemtray that the own main icon has been changed,
     * so should be reloaded
     */
    void NewIcon();

    /**
     * Inform the systemtray that there is a new icon to be used as overlay
     */
    void NewOverlayIcon();

    /**
     * Inform the systemtray that the requesting attention icon
     * has been changed, so should be reloaded
     */
    void NewAttentionIcon();

    /**
     * Inform the systemtray that something in the tooltip has been changed
     */
    void NewToolTip();

    /**
     * Signal the new status when it has been changed
     * @see Status
     */
    void NewStatus(const QString &status);

private:
    KNotificationItem *m_notificationItem;
    QString m_service;
    QDBusConnection m_dbus;
    static int s_serviceCount;
};

} // namespace Experimental

const QDBusArgument &operator<<(QDBusArgument &argument, const ExperimentalKDbusImageStruct &icon);
const QDBusArgument &operator>>(const QDBusArgument &argument, ExperimentalKDbusImageStruct &icon);

Q_DECLARE_METATYPE(ExperimentalKDbusImageStruct)

const QDBusArgument &operator<<(QDBusArgument &argument, const ExperimentalKDbusImageVector &iconVector);
const QDBusArgument &operator>>(const QDBusArgument &argument, ExperimentalKDbusImageVector &iconVector);

Q_DECLARE_METATYPE(ExperimentalKDbusImageVector)

const QDBusArgument &operator<<(QDBusArgument &argument, const ExperimentalKDbusToolTipStruct &toolTip);
const QDBusArgument &operator>>(const QDBusArgument &argument, ExperimentalKDbusToolTipStruct &toolTip);

Q_DECLARE_METATYPE(ExperimentalKDbusToolTipStruct)

#endif
