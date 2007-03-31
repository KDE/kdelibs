/* This file is part of the KDE libraries
   Copyright (C) 2000 Daniel M. Duley <mosfet@kde.org>
   Copyright (C) 2006 Olivier Goffart <ogoffart@kde.org>

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

#ifndef KMENU_H
#define KMENU_H

#include <kdeui_export.h>

#include <QtGui/QMenu>

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
    explicit KMenu(QWidget *parent = 0L);

    /**
     * Constructs a KMenu.
     * \param title The text displayed in a parent menu when it is inserted
     *              into another menu as a submenu.
     * \param parent the parent QWidget object
     */
    explicit KMenu(const QString& title, QWidget *parent = 0L);

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
     * \warning calls to text() of currently keyboard-selected items will
     * contain additional ampersand characters.
     *
     * \warning though pre-existing keyboard shortcuts will not interfere with the
     * operation of this feature, they may be confusing to the user as the existing
     * shortcuts will not work.  In addition, where text already contains ampersands,
     * the underline produced is likely to confuse the user (as this feature uses
     * underlining of text to indicate the current key selection sequence).
     */
    void setKeyboardShortcutsEnabled(bool enable);

    /**
     * Enables execution of the menu item once it is uniquely specified.
     * Defaults to off.
     */
    void setKeyboardShortcutsExecute(bool enable);

    /**
     * Returns the context menu associated with this menu
     * The data property of all actions inserted into the context menu is modified
     * all the time to point to the action and menu it has been shown for
     */
    QMenu* contextMenu();

    /**
     * Returns the context menu associated with this menu
     */
    const QMenu* contextMenu() const;

    /**
     * Hides the context menu if shown
     */
    void hideContextMenu();

    /**
     * Returns the KMenu associated with the current context menu
     */
    static KMenu* contextMenuFocus();

    /**
     * returns the QAction associated with the current context menu
     */
    static QAction* contextMenuFocusAction();

    /**
     * Return the state of the mouse buttons when the last menuitem was activated.
     */
    Qt::MouseButtons mouseButtons() const;

    /**
     * Return the state of the keyboard modifiers when the last menuitem was activated.
     */
    Qt::KeyboardModifiers keyboardModifiers() const;

Q_SIGNALS:
    /**
     * connect to this signal to be notified when a context menu is about to be shown
     * @param menu The menu that the context menu is about to be shown for
     * @param menuAction The action that the context menu is currently on
     * @param ctxMenu The context menu itself
     */
    void aboutToShowContextMenu(KMenu* menu, QAction* menuAction, QMenu* ctxMenu);

protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual bool focusNextPrevChild( bool next );
    virtual void contextMenuEvent(QContextMenuEvent *e);
    virtual void hideEvent(QHideEvent*);

private:
    QString underlineText(const QString& text, uint length);
    class KMenuPrivate;
    KMenuPrivate * const d;

    Q_PRIVATE_SLOT(d, void resetKeyboardVars(bool b = false))
    Q_PRIVATE_SLOT(d, void actionHovered(QAction*))
    Q_PRIVATE_SLOT(d, void showCtxMenu(const QPoint &))

};



#endif
