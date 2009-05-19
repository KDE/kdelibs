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

#ifndef KNOTIFICATIONITEN_H
#define KNOTIFICATIONITEN_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtDBus/QDBusArgument>
#include <QtCore/QPoint>

#include "knotificationitem_export.h"

class KActionCollection;
class KMenu;
class QMovie;

namespace Experimental
{

class KNotificationItemPrivate;

/**
 * \brief %KDE Notification icon protocol implementation
 *
 * This class implements the Notification icon Dbus specification.
 * It provides an icon similar to the classical systemtray icons,
 * with some key differences:
 *  the actual representation is done by the systemtray 
 *  (or the app behaving like it) itself, not by this app.
 *
 *  there is communication between the systemtray and the icon owner,
 *  so the system tray can know if the application is in a normal
 *  or in a requesting attention state
 *
 *  icons are divided in categories, so the systemtray can represent
 *  in a different way the icons from normal applications and for
 *  instance the ones about hardware status.
 *  @author Marco Martin <notmart@gmail.com>
 */
class KNOTIFICATIONITEM_EXPORT KNotificationItem : public QObject
{
    Q_OBJECT

    Q_ENUMS(ItemStatus)
    Q_ENUMS(ItemCategory)

    friend class KNotificationItemDBus;
    friend class KNotificationItemPrivate;
public:
    /**
     * All the possible status this icon can have, depending on the
     * importance of the events that happens in the parent application
     */
    enum ItemStatus {
        Passive = 1, /**Nothing is happening in the application, showing this icon is not required*/
        Active = 2 /**The application is doing something or is important that is always reachable from the user*/,
        NeedsAttention = 3 /**The application requests the attention of the user, for instance battery running out or a new IM message incoming*/
    };

    /**
     * Different kinds of applications announce their type to the systemtray,
     * so can be drawn in a different way or in a different place
     */
    enum ItemCategory {
        ApplicationStatus = 1, /**An icon for a normal application, can be seen as its taskbar entry*/
        Communications = 2 /**This is a communication oriented application this icon will be used for things such as the notification of a new message*/,
        SystemServices = 3 /**This is a system service, it can show itself in the system tray if it requires interaction from the user or wants to inform him about something*/,
        Hardware = 4 /**This application shows hardware status or a mean to control it*/
    };

    /**
     * Construct a new notification icon
     *
     * @arg parent the parent object for this object. If the object passed in as
     * a parent is also a QWidget, it will  be used as the main application window
     * represented by this icon and will be shown/hidden when an activation is requested.
     * @see associatedWidget
     * @see associatedWidget
     **/
    explicit KNotificationItem(QObject *parent = 0);

    /**
     * Construct a new notification icon with a unique identifier.
     * If your application has more than one notification item and the user
     * should be able to manipulate them separately (e.g. mark them for hiding
     * in a user interface), the id can be used to differentiate between them.
     *
     * The id should remain consistent even between application restarts.
     * Notification items without ids default to the application's name for the id.
     *
     * @arg id the unique id for this icon
     * @arg parent the parent object for this object. If the object passed in as
     * a parent is also a QWidget, it will  be used as the main application window
     * represented by this icon and will be shown/hidden when an activation is requested.
     * @see associatedWidget
     **/
    explicit KNotificationItem(const QString &id, QObject *parent = 0);

    ~KNotificationItem();

    /**
     * The id, which is guaranteed to be consistent between application starts and
     * untranslated, therefore useful for storing configuration related to this item.
     */
    QString id() const;

    /**
     * Sets the category for this icon, usually it's needed to call this function only once
     *
     * @arg category the new category for this icon
     */
    void setCategory(const ItemCategory category);

    /**
     * @return the application category
     */
    ItemCategory category() const;

    /**
     * Sets a title for this icon
     */
    void setTitle(const QString &title);

    /**
     * @return the title of this icon
     */
    QString title() const;

    /**
     * Sets a new status for this icon,
     * a newStatus will be emitted after this call
     * @see newStatus()
     */
    void setStatus(const ItemStatus status);

    /**
     * @return the current application status
     */
    ItemStatus status() const;

    //Main icon related functions
    /**
     * Sets a new main icon for the system tray
     *
     * @arg name it must be a KIconLoader compatible name, this is
     * the preferred way to set an icon
     */
    void setIcon(const QString &name);

    /**
     * @return the name of the main icon to be displayed
     * if image() is not empty this will always return an empty string
     */
    QString icon() const;

    /**
     * Sets a new main icon for the system tray
     *
     * @arg pixmap a QPixmap representing the icon, use setIcon(const QString) when possible
     */
    void setImage(const QIcon &icon);

    /**
     * @return a pixmap of the icon
     */
    QIcon image() const;

    /**
     * Sets an icon to be used as overlay for the main one
     * @arg icon name
     */
    void setOverlayIcon(const QString &name);

    /**
     * @return the name of the icon to be used as overlay fr the main one
     */
    QString overlayIcon() const;

    /**
     * Sets an icon to be used as overlay for the main one
     *
     * @arg pixmap a QPixmap representing the icon, use setOverlayIcon(const QString) when possible
     */
    void setOverlayImage(const QIcon &icon);

    /**
     * @return a pixmap of the icon
     */
    QIcon overlayImage() const;

    //Requesting attention icon

    /**
     * Sets a new icon that should be used when the application
     * wants to request attention (usually the systemtray
     * will blink between this icon and the main one)
     *
     * @arg pixmap a QPixmap representing the icon, use setIcon(const QString) when possible
     */
    void setAttentionIcon(const QString &name);

    /**
     * @return the name of the icon to be displayed when the application
     * is requesting the user's attention
     * if attentionImage() is not empty this will always return an empty string
     */
    QString attentionIcon() const;

    /**
     * Sets the pixmap of the requesting attention icon.
     * Use setAttentionIcon(const QString) instead when possible.
     */
    void setAttentionImage(const QIcon &icon);

    /**
     * @return a pixmap of the requesting attention icon
     */
    QIcon attentionImage() const;

    /**
     * Sets a movie as the requesting attention icon.
     * This overrides anything set in setAttentionIcon() or setAttentionImage()
     */
    void setAttentionMovie(QMovie *movie);

    //FIXME: these two will die...
    /**
     * Sets a movie represented by a vector of QPixmap as frames
     * This is an overloaded member provided for convenience
     */
    void setAttentionMovie(const QVector<QPixmap> &movie);

    /**
     * Sets a movie represented by a vector of QImage as frames
     * This is an overloaded member provided for convenience
     */
    void setAttentionMovie(const QVector<QImage> &movie);

    /**
     * @return the movie
     */
    QMovie *attentionMovie() const;


    //ToolTip handling
    /**
     * Sets a new toolTip or this icon, a toolTip is composed of an icon,
     * a title ad a text, all fields are optional.
     * newToolTip() signal will be emitted
     * @see newToolTip()
     *
     * @arg iconName a KIconLoader compatible name for the tootip icon
     * @arg title tootip title
     * @arg subTitle subtitle for the toolTip
     */
    void setToolTip(const QString &iconName, const QString &title, const QString &subTitle);

    /**
     * Sets a new toolTip or this icon, using a QPixmap as an icon instead of the name.
     * newToolTip() signal will be emitted
     * @see newToolTip()
     *
     * If possible setToolTip(const QString &, const QString &, const QString &) should be used.
     */
    void setToolTip(const QPixmap &icon, const QString &title, const QString &subTitle);

    /**
     * Sets a new toolTip or this notification icon.
     * newToolTip() signal will be emitted
     * @see newToolTip()
     * This is an overloaded member provided for convenience
     */
    void setToolTip(const QIcon &icon, const QString &title, const QString &subTitle);

    /**
     * Set a new icon for the toolTip
     * newToolTip() signal will be emitted
     * @see newToolTip()
     *
     * @arg name the name for the icon
     */
    void setToolTipIcon(const QString &name);

    /**
     * @return the name of the toolTip icon
     * if toolTipImage() is not empty this will always return an empty string
     */
    QString toolTipIcon() const;

    /**
     * Set a new icon for the toolTip.
     * newToolTip() signal will be emitted
     * @see newToolTip()
     *
     * Use setToolTipIcon(QPixmap) if possible.
     * @arg pixmap representing the icon
     */
    void setToolTipImage(const QIcon &icon);

    /**
     * @return a serialization of the toolTip icon data
     */
    QIcon toolTipImage() const;

    /**
     * Sets a new title for the toolTip
     * newToolTip() signal will be emitted
     * @see newToolTip()
     */
    void setToolTipTitle(const QString &title);

    /**
     * @return the title of the main icon toolTip
     */
    QString toolTipTitle() const;

    /**
     * Sets a new subtitle for the toolTip
     * newToolTip() signal will be emitted
     * @see newToolTip()
     */
    void setToolTipSubTitle(const QString &subTitle);

    /**
     * @return the subtitle of the main icon toolTip
     */
    QString toolTipSubTitle() const;

    /**
     * Sets a new context menu for this NotificationIcon.
     * the menu will be shown with a contextMenu(int,int)
     * call by the systemtray over dbus
     * usually you don't need to call this unless you want to use
     * a custom KMenu subclass as context menu
     * @see contextMenu(int, int)
     */
    void setContextMenu(KMenu *menu);

    /**
     * Access the context menu associated to this notification icon
     */
    KMenu *contextMenu() const;

    /**
     * Sets the main widget associated with this NotificationIcon
     */
    void setAssociatedWidget(QWidget *parent);

    /**
     * Access the main widget associated with this NotificationIcon
     */
    QWidget *associatedWidget() const;

    /**
     * All the actions present in the menu
     */
    KActionCollection *actionCollection() const;

    void showMessage(const QString &title, const QString &message, const QString &icon, int timeout = 10000);


public Q_SLOTS:

    /**
     * Shows the main widget and try to position it on top
     * of the other windows, if the widget is already visible, hide it.
     */
    virtual void activate(const QPoint &pos = QPoint());

Q_SIGNALS:
    /**
     * Inform the host application that the mouse wheel has been used
     */
    void scrollRequested(int delta, Qt::Orientation orientation);

    /**
     * Inform the host application that an activation has been requested
     */
    void activateRequested(bool active, const QPoint &pos);

    /**
     * Alternate activate action
     */
    void secondaryActivateRequested(const QPoint &pos);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    KNotificationItemPrivate *const d;

    Q_PRIVATE_SLOT(d, void serviceChange(const QString& name,
                                         const QString& oldOwner,
                                         const QString& newOwner))
    Q_PRIVATE_SLOT(d, void registerToDaemon())
    Q_PRIVATE_SLOT(d, void contextMenuAboutToShow())
    Q_PRIVATE_SLOT(d, void maybeQuit())
    Q_PRIVATE_SLOT(d, void minimizeRestore())
    Q_PRIVATE_SLOT(d, void hideMenu())
    Q_PRIVATE_SLOT(d, void legacyWheelEvent(int))
    Q_PRIVATE_SLOT(d, void legacyActivated(QSystemTrayIcon::ActivationReason))
};

} // namespace Experimental

#endif
