/* This file is part of the KDE libraries
   Copyright (C) 2000 Daniel M. Duley <mosfet@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _KMENU_H
#define _KMENU_H

#include <QMenu>
#include <QPointer>
#include <QAction>
#include <kpixmapeffect.h>
#include <kpixmap.h>
#include <kdelibs_export.h>


/**
 * @short A menu with keyboard searching
 *
 * KMenu is a class for menus with  keyboard
 * accessibility for popups with many options and/or varying options. It acts
 * identically to QMenu, with the addition of setKeyboardShortcutsEnabled() and
 * setKeyboardShortcutsExecute() methods.
 *
 *
 * The keyboard search algorithm is incremental with additional underlining
 * for user feedback.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 * @author Hamish Rodda <rodda@kde.org>
 */
class KDEUI_EXPORT KMenu : public QMenu {
    Q_OBJECT
public:
    /**
     * Constructs a KMenu.
     */
    KMenu(QWidget *parent = 0L);

    /**
     * Constructs a KMenu.
     * \param title The text displayed in a parent menu when it is inserted
     *              into another menu as a submenu.
     * \param parent the parent QWidget object
     */
    KMenu(const QString& title, QWidget *parent = 0L);

    /**
     * Destructs the object
     */
    ~KMenu();

    /**
     * Inserts a title item with no icon.
     */
    QAction* addTitle(const QString &text, QAction* before = 0L);

    /**
     * Inserts a title item with the given icon and title.
     */
    QAction* addTitle(const QIcon &icon, const QString &text, QAction* before = 0L);

    /**
     * Enables keyboard navigation by searching for the entered key sequence.
     * Also underlines the currently selected item, providing feedback on the search.
     *
     * Defaults to off.
     *
     * WARNING: calls to text() of currently keyboard-selected items will
     * contain additional ampersand characters.
     *
     * WARNING: though pre-existing keyboard shortcuts will not interfere with the
     * operation of this feature, they may be confusing to the user as the existing
     * shortcuts will not work.
     * @since 3.1
     */
    void setKeyboardShortcutsEnabled(bool enable);

    /**
     * Enables execution of the menu item once it is uniquely specified.
     * Defaults to off.
     * @since 3.1
     */
    void setKeyboardShortcutsExecute(bool enable);

    /**
     * Returns the context menu associated with this menu
     * The data property of all actions inserted into the context menu is modified all the time to point to the action and menu it has been shown for
     */
    QMenu* contextMenu();

    /**
     * Returns the context menu associated with this menu
     */
    const QMenu* contextMenu() const;

    /**
     * Hides the context menu if shown
     * @since 3.2
     */
    void hideContextMenu();

    /**
     * Returns the KMenu associated with the current context menu
     * @since 3.2
     */
    static KMenu* contextMenuFocus();

    /**
     * returns the QAction associated with the current context menu
     * @since 3.2
     */
    static QAction* contextMenuFocusAction();

    /**
     * Return the state of the mouse buttons when the last menuitem was activated.
     * @since 4.0
     */
    Qt::MouseButtons mouseButtons() const;

    /**
     * Return the state of the keyboard modifiers when the last menuitem was activated.
     * @since 4.0
     */
    Qt::KeyboardModifiers keyboardModifiers() const;

signals:
    /**
     * connect to this signal to be notified when a context menu is about to be shown
     * @param menu The menu that the context menu is about to be shown for
     * @param menuAction The action that the context menu is currently on
     * @param ctxMenu The context menu itself
     * @since 3.2
     */
    void aboutToShowContextMenu(KMenu* menu, QAction* menuAction, QMenu* ctxMenu);

protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void keyPressEvent(QKeyEvent* e);
    /// @since 3.4
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual bool focusNextPrevChild( bool next );
    virtual void contextMenuEvent(QContextMenuEvent *e);
    virtual void hideEvent(QHideEvent*);

    virtual void virtual_hook( int id, void* data );

protected slots:
    /// @since 3.1
    QString underlineText(const QString& text, uint length);
    /// @since 3.1
    void resetKeyboardVars(bool noMatches = false);
    void actionHovered(QAction* action);
    void showCtxMenu(const QPoint &pos);
    void ctxMenuHiding();
    void ctxMenuHideShowingMenu();

private:
    class KMenuPrivate;
    KMenuPrivate *d;
};

/**
 * custom variant type for QAction::data of kmenu context menus
 * @author Joseph Wenninger <jowenn@kde.org>
 */
class KDEUI_EXPORT KMenuContext {
public:
    KMenuContext();
    KMenuContext(const KMenuContext& o);
    KMenuContext(QPointer<KMenu> menu,QPointer<QAction> action);

    inline QPointer<KMenu> menu() const { return m_menu; }
    inline QPointer<QAction> action() const { return m_action; }

private:
    QPointer<KMenu> m_menu;
    QPointer<QAction> m_action;
};



#endif
