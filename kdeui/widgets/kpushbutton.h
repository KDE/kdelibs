/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KPUSHBUTTON_H
#define KPUSHBUTTON_H

#include <QtGui/QPushButton>

#include <kstandardguiitem.h>

class QDrag;
class QMenu;
class KIcon;

namespace KAuth {
    class Action;
}

/**
 * @brief A QPushButton with drag-support and KGuiItem support
 *
 * This is nothing but a QPushButton with drag-support and KGuiItem support.
 * You must call #setDragEnabled (true) and override the virtual method
 * dragObject() to specify the QDragObject to be used.
 *
 * \image html kpushbutton.png "KDE Push Button"
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KDEUI_EXPORT KPushButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isDragEnabled READ isDragEnabled WRITE setDragEnabled)

public:

    /**
     * Default constructor.
     */
    explicit KPushButton( QWidget *parent = 0 );

    /**
     * Constructor, that sets the button-text to @p text
     */
    explicit KPushButton( const QString &text, QWidget *parent = 0 );

    /**
     * Constructor, that sets an icon and the button-text to @p text
     */
    KPushButton( const KIcon &icon, const QString &text, QWidget *parent = 0 );

    /**
     * Constructor that takes a KGuiItem for the text, the icon, the tooltip
     * and the what's this help
     */
    explicit KPushButton( const KGuiItem &item, QWidget *parent = 0 );

    /**
     * Destructs the button.
     */
    ~KPushButton();

    /**
     * Enables/disables drag-support. Default is disabled.
     */
    void setDragEnabled( bool enable );

    /**
     * @returns if drag support is enabled or not.
     */
    bool isDragEnabled() const;

    /**
     * Sets the KGuiItem for this button.
     */
    void setGuiItem( const KGuiItem& item );

    /**
    * Sets the standard KGuiItem for this button.
    */
    void setGuiItem( KStandardGuiItem::StandardItem item );

    /**
     * Reads the standard KGuiItem for this button.
     */
    KStandardGuiItem::StandardItem guiItem() const;

    /**
     * Sets the Icon Set for this button. It also takes into account the
     * KGlobalSettings::showIconsOnPushButtons() setting.
     */
    void setIcon( const KIcon &icon );

    /**
     * Sets the pixmap for this button. Rarely used. This one exists mostly for usage in Qt designer,
     * with icons embedded into the ui file. But you should rather save them separately, and load them
     * with KIcon("name") so that the icons are themeable.
     */
    KDE_DEPRECATED void setIcon( const QIcon &pix );

    /**
    * Sets the text of the button
    */
    void setText( const QString &text );

    /**
     * Sets a delayed popup menu
     * for consistency, since menu() isn't virtual
     */
     void setDelayedMenu(QMenu *delayed_menu);

    /**
     * returns a delayed popup menu
     * since menu() isn't virtual
     */
     QMenu *delayedMenu();

    /**
     * Reimplemented to add arrow for delayed menu
     * @since 4.4
     */
    virtual QSize sizeHint() const;

    /**
     * Returns the action object associated with this button, or 0 if it does not have one
     *
     * @returns the KAuth::Action associated with this button.
     */
     KAuth::Action *authAction() const;

    /**
     * Sets the action object associated with this button
     *
     * By setting a KAuth::Action, this button will become associated with it, and
     * whenever it gets clicked, it will trigger the authorization and execution process
     * for the action. The signal activated will also be emitted whenever the button gets
     * clicked and the action gets authorized. Pass 0 to this function to disassociate the button
     *
     * @param action the KAuth::Action to associate with this button.
     */
     void setAuthAction(KAuth::Action *action);

     /**
     * Sets the action object associated with this button
     *
     * Overloaded member to allow creating the action by name
     *
     * @param actionName the name of the action to associate
     */
     void setAuthAction(const QString &actionName);

protected:
    /**
     * Reimplement this and return the QDrag object that should be used
     * for the drag. Remember to give it "this" as parent.
     *
     * Default implementation returns 0, so that no drag is initiated.
     */
    virtual QDrag * dragObject();

    /**
     * Reimplemented to add drag-support
     */
    virtual void mousePressEvent( QMouseEvent * );
    /**
     * Reimplemented to add drag-support
     */
    virtual void mouseMoveEvent( QMouseEvent * );

    /**
     * Reimplemented to add arrow for delayed menu
     * @since 4.4
     */
    virtual void paintEvent( QPaintEvent * );

    /**
     * Starts a drag (dragCopy() by default) using dragObject()
     */
    virtual void startDrag();

Q_SIGNALS:
    /**
     * Signal emitted when the button is triggered and authorized
     *
     * If the button needs authorization, whenever the user triggers it,
     * the authorization process automatically begins.
     * If it succeeds, this signal is emitted. The KAuth::Action object is provided for convenience
     * if you have multiple Action objects, but of course it's always the same set with
     * setAuthAction().
     *
     * WARNING: If your button needs authorization you should connect eventual slots processing
     * stuff to this signal, and NOT clicked. Clicked will be emitted even if the user has not
     * been authorized
     *
     * @param action The object set with setAuthAction()
     */
    void authorized(KAuth::Action *action);

private:
    /**
     * Internal.
     * Initialize the KPushButton instance
     */
    void init( const KGuiItem &item );

private:
    class KPushButtonPrivate;
    KPushButtonPrivate * const d;

    Q_PRIVATE_SLOT(d, void slotSettingsChanged( int ))
    Q_PRIVATE_SLOT(d, void slotPressedInternal())
    Q_PRIVATE_SLOT(d, void slotClickedInternal())
    Q_PRIVATE_SLOT(d, void slotDelayedMenuTimeout())
    Q_PRIVATE_SLOT(d, void authStatusChanged(int))
};

#endif // KPUSHBUTTON_H
