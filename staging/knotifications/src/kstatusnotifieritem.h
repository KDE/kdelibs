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

#ifndef KSTATUSNOTIFIERITEM_H
#define KSTATUSNOTIFIERITEM_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtDBus/QDBusArgument>
#include <QtCore/QPoint>

#include "kdeui_export.h"

class QAction;
class QMenu;

class KStatusNotifierItemPrivate;

/**
 * \brief %KDE Status notifier Item protocol implementation
 *
 * This class implements the Status notifier Item Dbus specification.
 * It provides an icon similar to the classical systemtray icons,
 * with some key differences:
 *
 * - the actual representation is done by the systemtray (or the app behaving
 *   like it) itself, not by this app.  Since 4.5 this also includes the menu,
 *   which means you cannot use embed widgets in the menu.
 *
 * - there is communication between the systemtray and the icon owner, so the
 *   system tray can know if the application is in a normal or in a requesting
 *   attention state.
 *
 * - icons are divided in categories, so the systemtray can represent in a
 *   different way the icons from normal applications and for instance the ones
 *   about hardware status.
 *
 * Whenever possible you should prefer passing icon by name rather than by
 * pixmap because:
 *  
 * - it is much lighter on Dbus (no need to pass all image pixels).
 *
 * - it makes it possible for the systemtray to load an icon of the appropriate
 *   size or to replace your icon with a systemtray specific icon which matches
 *   with the desktop theme.
 *
 * - some implementations of the system tray do not support passing icons by
 *   pixmap and will show a blank icon instead.
 *
 * @author Marco Martin <notmart@gmail.com>
 * @since 4.4
 */
class KDEUI_EXPORT KStatusNotifierItem : public QObject
{
    Q_OBJECT

    Q_ENUMS(ItemStatus)
    Q_ENUMS(ItemCategory)
    Q_PROPERTY( ItemCategory category READ category WRITE setCategory )
    Q_PROPERTY( QString title READ title WRITE setTitle )
    Q_PROPERTY( ItemStatus status READ status WRITE setStatus )
    Q_PROPERTY( QString iconName READ iconName WRITE setIconByName )
    Q_PROPERTY( QString overlayIconName READ overlayIconName WRITE setOverlayIconByName )
    Q_PROPERTY( QString attentionIconName READ attentionIconName WRITE setAttentionIconByName )
    Q_PROPERTY( QString toolTipIconName READ toolTipIconName WRITE setToolTipIconByName )
    Q_PROPERTY( QString toolTipTitle READ toolTipTitle WRITE setToolTipTitle )
    Q_PROPERTY( QString toolTipSubTitle READ toolTipSubTitle WRITE setToolTipSubTitle )

    friend class KStatusNotifierItemDBus;
    friend class KStatusNotifierItemPrivate;
public:
    /**
     * All the possible status this icon can have, depending on the
     * importance of the events that happens in the parent application
     */
    enum ItemStatus {
        /// Nothing is happening in the application, so showing this icon is not required
        Passive = 1,
        /// The application is doing something, or it is important that the
        /// icon is always reachable from the user
        Active = 2,
        /// The application requests the attention of the user, for instance
        /// battery running out or a new IM message was received
        NeedsAttention = 3
    };

    /**
     * Different kinds of applications announce their type to the systemtray,
     * so can be drawn in a different way or in a different place
     */
    enum ItemCategory {
        /// An icon for a normal application, can be seen as its taskbar entry
        ApplicationStatus = 1,
        /// This is a communication oriented application; this icon will be used
        /// for things such as the notification of a new message
        Communications = 2,
        /// This is a system service, it can show itself in the system tray if
        /// it requires interaction from the user or wants to inform him about
        /// something
        SystemServices = 3,
        /// This application shows hardware status or a means to control it
        Hardware = 4,
        Reserved = 129
    };

    /**
     * Construct a new status notifier item
     *
     * @param parent the parent object for this object. If the object passed in as
     * a parent is also a QWidget, it will  be used as the main application window
     * represented by this icon and will be shown/hidden when an activation is requested.
     * @see associatedWidget
     **/
    explicit KStatusNotifierItem(QObject *parent = 0);

    /**
     * Construct a new status notifier item with a unique identifier.
     * If your application has more than one status notifier item and the user
     * should be able to manipulate them separately (e.g. mark them for hiding
     * in a user interface), the id can be used to differentiate between them.
     *
     * The id should remain consistent even between application restarts.
     * Status notifier items without ids default to the application's name for the id.
     * This id may be used, for instance, by hosts displaying status notifier items to 
     * associate configuration information with this item in a way that can persist
     * between sessions or application restarts.
     *
     * @param id the unique id for this icon
     * @param parent the parent object for this object. If the object passed in as
     * a parent is also a QWidget, it will  be used as the main application window
     * represented by this icon and will be shown/hidden when an activation is requested.
     * @see associatedWidget
     **/
    explicit KStatusNotifierItem(const QString &id, QObject *parent = 0);

    ~KStatusNotifierItem();

    /**
     * @return The id which was specified in the constructor. This should be
     * guaranteed to be consistent between application starts and
     * untranslated, as host applications displaying items may use it for
     * storing configuration related to this item.
     */
    QString id() const;

    /**
     * Sets the category for this icon, usually it's needed to call this function only once
     *
     * @param category the new category for this icon
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
     * Sets a new status for this icon.
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
     * @param name it must be a KIconLoader compatible name, this is
     * the preferred way to set an icon
     */
    void setIconByName(const QString &name);

    /**
     * @return the name of the main icon to be displayed
     * if image() is not empty this will always return an empty string
     */
    QString iconName() const;

    /**
     * Sets a new main icon for the system tray
     *
     * @param pixmap our icon, use setIcon(const QString) when possible
     */
    void setIconByPixmap(const QIcon &icon);

    /**
     * @return a pixmap of the icon
     */
    QIcon iconPixmap() const;

    /**
     * Sets an icon to be used as overlay for the main one
     * @param icon name, if name is and empty QString()
     *     (and overlayIconPixmap() is empty too) the icon will be removed
     */
    void setOverlayIconByName(const QString &name);

    /**
     * @return the name of the icon to be used as overlay fr the main one
     */
    QString overlayIconName() const;

    /**
     * Sets an icon to be used as overlay for the main one
     *   setOverlayIconByPixmap(QIcon()) will remove the overlay when
     *   overlayIconName() is empty too.
     *
     * @param pixmap our overlay icon, use setOverlayIcon(const QString) when possible.
     */
    void setOverlayIconByPixmap(const QIcon &icon);

    /**
     * @return a pixmap of the icon
     */
    QIcon overlayIconPixmap() const;

    //Requesting attention icon

    /**
     * Sets a new icon that should be used when the application
     * wants to request attention (usually the systemtray
     * will blink between this icon and the main one)
     *
     * @param name KIconLoader-compatible name of icon to use
     */
    void setAttentionIconByName(const QString &name);

    /**
     * @return the name of the icon to be displayed when the application
     * is requesting the user's attention
     * if attentionImage() is not empty this will always return an empty string
     */
    QString attentionIconName() const;

    /**
     * Sets the pixmap of the requesting attention icon.
     * Use setAttentionIcon(const QString) instead when possible.
     *
     * @param icon QIcon to use for requesting attention.
     */
    void setAttentionIconByPixmap(const QIcon &icon);

    /**
     * @return a pixmap of the requesting attention icon
     */
    QIcon attentionIconPixmap() const;

    /**
     * Sets a movie as the requesting attention icon.
     * This overrides anything set in setAttentionIcon()
     */
    void setAttentionMovieByName(const QString &name);

    /**
     * @return the name of the movie to be displayed when the application is
     * requesting the user attention
     */
    QString attentionMovieName() const;


    //ToolTip handling
    /**
     * Sets a new toolTip or this icon, a toolTip is composed of an icon,
     * a title ad a text, all fields are optional.
     *
     * @param iconName a KIconLoader compatible name for the tootip icon
     * @param title tootip title
     * @param subTitle subtitle for the toolTip
     */
    void setToolTip(const QString &iconName, const QString &title, const QString &subTitle);

    /**
     * Sets a new toolTip or this status notifier item.
     * This is an overloaded member provided for convenience
     */
    void setToolTip(const QIcon &icon, const QString &title, const QString &subTitle);

    /**
     * Set a new icon for the toolTip
     *
     * @param name the name for the icon
     */
    void setToolTipIconByName(const QString &name);

    /**
     * @return the name of the toolTip icon
     * if toolTipImage() is not empty this will always return an empty string
     */
    QString toolTipIconName() const;

    /**
     * Set a new icon for the toolTip.
     *
     * Use setToolTipIconByName(QString) if possible.
     * @param pixmap representing the icon
     */
    void setToolTipIconByPixmap(const QIcon &icon);

    /**
     * @return a serialization of the toolTip icon data
     */
    QIcon toolTipIconPixmap() const;

    /**
     * Sets a new title for the toolTip
     */
    void setToolTipTitle(const QString &title);

    /**
     * @return the title of the main icon toolTip
     */
    QString toolTipTitle() const;

    /**
     * Sets a new subtitle for the toolTip
     */
    void setToolTipSubTitle(const QString &subTitle);

    /**
     * @return the subtitle of the main icon toolTip
     */
    QString toolTipSubTitle() const;

    /**
     * Sets a new context menu for this StatusNotifierItem.
     * the menu will be shown with a contextMenu(int,int)
     * call by the systemtray over dbus
     * usually you don't need to call this unless you want to use
     * a custom QMenu subclass as context menu
     */
    void setContextMenu(QMenu *menu);

    /**
     * Access the context menu associated to this status notifier item
     */
    QMenu *contextMenu() const;

    /**
     * Sets the main widget associated with this StatusNotifierItem
     *
     * If you pass contextMenu() as a parent then the menu will be displayed
     * when the user activate the icon. In this case the activate() method will
     * not be called and the activateRequested() signal will not be emitted
     *
     * @param parent the new main widget: must be a top level window,
     *               if it's not parent->window() will be used instead.
     */
    void setAssociatedWidget(QWidget *parent);

    /**
     * Access the main widget associated with this StatusNotifierItem
     */
    QWidget *associatedWidget() const;

    /**
     * All the actions present in the menu
     */
    QList<QAction *> actionCollection() const;

    /**
     * Adds an action to the actionCollection()
     *
     * @param name the name of the action
     * @param action the action we want to add
     */
    void addAction(const QString &name, QAction *action);

    /**
     * Removes an action to the collection
     *
     * @param name the name of the action
     */
    void removeAction(const QString &name);

    /**
     * Sets whether to show the standard items in the menu, such as Quit
     */
    void setStandardActionsEnabled(bool enabled);

    /**
     * @return if the standard items in the menu, such as Quit
     */
    bool standardActionsEnabled() const;

    /**
     * Shows the user a notification. If possible use KNotify instead
     *
     * @param title message title
     * @param message the actual text shown to the user
     * @param icon icon to be shown to the user
     * @param timeout how much time will elaps before hiding the message
     */
    void showMessage(const QString &title, const QString &message, const QString &icon, int timeout = 10000);


public Q_SLOTS:

    /**
     * Shows the main widget and try to position it on top
     * of the other windows, if the widget is already visible, hide it.
     *
     * @param pos if it's a valid position it represents the mouse coordinates when the event was triggered
     */
    virtual void activate(const QPoint &pos = QPoint());

Q_SIGNALS:
    /**
     * Inform the host application that the mouse wheel
     * (or another mean of scrolling that the visualization provides) has been used
     *
     * @param delta the amount of scrolling, can be either positive or negative
     * @param orientation direction of the scrolling, can be either horizontal or vertical
     */
    void scrollRequested(int delta, Qt::Orientation orientation);

    /**
     * Inform the host application that an activation has been requested,
     *           for instance left mouse click, but this is not guaranteed since
     *           it's dependent from the visualization
     * @param active if it's true the application asked for the activatin
     *              of the main window, if it's false it asked for hiding
     * @param pos the position in the screen where the user clicked to
     *  trigger this signal, QPoint() if it's not the consequence of a mouse click.
     */
    void activateRequested(bool active, const QPoint &pos);

    /**
     * Alternate activate action,
     * for instance right mouse click, but this is not guaranteed since
     * it's dependent from the visualization
     *
     * @param pos the position in the screen where the user clicked to
     *  trigger this signal, QPoint() if it's not the consequence of a mouse click.
     */
    void secondaryActivateRequested(const QPoint &pos);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    KStatusNotifierItemPrivate *const d;

    Q_PRIVATE_SLOT(d, void serviceChange(const QString& name,
                                         const QString& oldOwner,
                                         const QString& newOwner))
    Q_PRIVATE_SLOT(d, void checkForRegisteredHosts())
    Q_PRIVATE_SLOT(d, void registerToDaemon())
    Q_PRIVATE_SLOT(d, void contextMenuAboutToShow())
    Q_PRIVATE_SLOT(d, void maybeQuit())
    Q_PRIVATE_SLOT(d, void minimizeRestore())
    Q_PRIVATE_SLOT(d, void hideMenu())
    Q_PRIVATE_SLOT(d, void legacyWheelEvent(int))
    Q_PRIVATE_SLOT(d, void legacyActivated(QSystemTrayIcon::ActivationReason))
};

#endif
