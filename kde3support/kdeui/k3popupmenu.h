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

#ifndef _K3POPUPMENU_H
#define _K3POPUPMENU_H

#ifndef QT3_SUPPORT
#define QT3_SUPPORT
#endif

#include <kde3support_export.h>

#include <Qt3Support/Q3PopupMenu>
#include <QtGui/QMenuItem>

/**
 * @short A menu with keyboard searching and convenience methods for title items.
 *
 * K3PopupMenu is a compatibility class for KPopupMenu from KDE 3.
 * It provides menus with standard title items and keyboard
 * accessibility for popups with many options and/or varying options. It acts
 * identically to QMenu, with the addition of insertTitle(),
 * changeTitle(), setKeyboardShortcutsEnabled() and
 * setKeyboardShortcutsExecute() methods.
 *
 * The titles support a text string and an icon.
 *
 * The keyboard search algorithm is incremental with additional underlining
 * for user feedback.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 * @author Hamish Rodda <rodda@kde.org>
 */
class KDE3SUPPORT_EXPORT K3PopupMenu : public Q3PopupMenu {
    Q_OBJECT
public:
    /**
     * Constructs a K3PopupMenu.
     */
    K3PopupMenu(QWidget *parent=0);

    /**
     * Destructs the object
     */
    ~K3PopupMenu();

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
     */
    void setKeyboardShortcutsEnabled(bool enable);

    /**
     * Enables execution of the menu item once it is uniquely specified.
     * Defaults to off.
     */
    void setKeyboardShortcutsExecute(bool enable);

    /**
     * Returns the context menu associated with this menu
     */
    Q3PopupMenu* contextMenu();

    /**
     * Returns the context menu associated with this menu
     */
    const Q3PopupMenu* contextMenu() const;

    /**
     * Hides the context menu if shown
     */
    void hideContextMenu();

    /**
     * Returns the K3PopupMenu associated with the current context menu
     */
    static K3PopupMenu* contextMenuFocus();

    /**
     * returns the QAction associated with the current context menu
     */
    static QAction* contextMenuFocusAction();

#ifdef QT3_SUPPORT
    /**
     * Return the state of the mouse button and keyboard modifiers
     * when the last menuitem was activated.
     */
    Qt::ButtonState state() const;
#endif

    /**
     * Return the state of the mouse buttons when the last menuitem was activated.
     */
    Qt::MouseButtons mouseButtons() const;

    /**
     * Return the state of the keyboard modifiers when the last menuitem was activated.
     */
    Qt::KeyboardModifiers keyboardModifiers() const;

#ifdef QT3_SUPPORT
    /**
     * Inserts a title item with no icon.
     */
    int insertTitle(const QString &text, int id=-1, int index=-1);
    /**
     * Inserts a title item with the given icon and title.
     */
    int insertTitle(const QPixmap &icon, const QString &text, int id=-1,
                    int index=-1);
    /**
     * Changes the title of the item at the specified id. If a icon was
     * previously set it is cleared.
     */
    void changeTitle(int id, const QString &text);
    /**
     * Changes the title and icon of the title item at the specified id.
     */
    void changeTitle(int id, const QPixmap &icon, const QString &text);
    /**
     * Returns the title of the title item at the specified id. The default
     * id of -1 is for backwards compatibility only, you should always specify
     * the id.
     */
    QString title(int id=-1) const;
    /**
     * Returns the icon of the title item at the specified id.
     */
    QPixmap titlePixmap(int id) const;

    /**
     * @deprecated
     * Obsolete method provided for backwards compatibility only. Use the
     * normal constructor and insertTitle instead.
     */
    K3PopupMenu(const QString &title, QWidget *parent=0);

    /**
     * @deprecated
     * Obsolete method provided for backwards compatibility only. Use
     * insertTitle and changeTitle instead.
     */
    void setTitle(const QString &title);

    /**
     * returns the ID of the menuitem associated with the current context menu
     */
    static int contextMenuFocusItem();

    /**
     * Reimplemented for internal purposes
     */
    virtual void activateItemAt(int index);
    // END compat methods

    /**
     * Helper for porting things. Returns ID of action, or -1 if passed null.
     * ### KDE4: should be able to remove on Monday
     */
    static int actionId(QAction* action)
    {
        if (!action)
            return -1;
        return static_cast<QMenuItem*>(action)->id();
    }
#endif
Q_SIGNALS:
    /**
     * connect to this signal to be notified when a context menu is about to be shown
     * @param menu The menu that the context menu is about to be shown for
     * @param menuAction The action that the context menu is currently on
     * @param ctxMenu The context menu itself
     */
    void aboutToShowContextMenu(K3PopupMenu* menu, QAction* menuAction, QMenu* ctxMenu);
    /// compat
    void aboutToShowContextMenu(K3PopupMenu* menu, int menuItem, Q3PopupMenu* ctxMenu);

protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual bool focusNextPrevChild( bool next );
    virtual void contextMenuEvent(QContextMenuEvent *e);
    virtual void hideEvent(QHideEvent*);

    virtual void virtual_hook( int id, void* data );

protected Q_SLOTS:
    QString underlineText(const QString& text, uint length);
    void resetKeyboardVars(bool noMatches = false);
    void actionHovered(QAction* action);
    void showCtxMenu(const QPoint &pos);
    void ctxMenuHiding();
    void ctxMenuHideShowingMenu();

private:
    class K3PopupMenuPrivate;
    K3PopupMenuPrivate *d;
    Q_DISABLE_COPY( K3PopupMenu )
};

#endif
