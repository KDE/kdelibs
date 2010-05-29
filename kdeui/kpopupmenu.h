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
#ifndef _KPOPUP_H
#define _KPOPUP_H "$Id: kpopupmenu.h 465272 2005-09-29 09:47:40Z mueller $"

#define INCLUDE_MENUITEM_DEF

#include <qpopupmenu.h>
#include <kpixmapeffect.h>
#include <kpixmap.h>
#include <kdelibs_export.h>
/**
 * @short KPopupMenu title widget.
 *
 * Title widget for use in KPopupMenu.
 *
 * You usually don't have to create this manually since
 * KPopupMenu::insertTitle will do it for you, but it is allowed if
 * you wish to customize it's look.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class KDEUI_EXPORT KPopupTitle : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructs a title widget with the user specified gradient, pixmap,
     * and colors.
     */
    KPopupTitle(QWidget *parent=0, const char *name=0);
    /**
     * @deprecated
     * Constructs a title widget with the specified gradient and colors.
     */
    KPopupTitle(KPixmapEffect::GradientType gradient, const QColor &color,
                const QColor &textColor, QWidget *parent=0,
                const char *name=0) KDE_DEPRECATED;
    /**
     * @deprecated
     * Constructs a title widget with the specified pixmap and colors.
     */
    KPopupTitle(const KPixmap &background, const QColor &color,
                const QColor &textColor, QWidget *parent=0,
                const char *name=0) KDE_DEPRECATED;
    /**
     * Sets the title string and optional icon for the title widget.
     *
     * You will want to call this before inserting into a menu.
     */
    void setTitle(const QString &text, const QPixmap *icon=0);
    /**
     * Returns the current title.
     */
    QString title() const { return titleStr; }
    /**
     * Returns the current icon.
     */
    QPixmap icon() const { return miniicon; }

    QSize sizeHint() const;

public slots:
    /// @since 3.1
    void setText( const QString &text );
    /// @since 3.1
    void setIcon( const QPixmap &pix );

protected:
    void calcSize();
    void paintEvent(QPaintEvent *ev);

    // Remove in KDE4
    KPixmapEffect::GradientType grType;
    QString titleStr;
    // Remove in KDE4
    KPixmap fill;
    QPixmap miniicon;
    QColor fgColor, bgColor, grHigh, grLow;
    bool useGradient;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KPopupTitlePrivate;
    KPopupTitlePrivate *d;
};

/**
 * @short A menu with title items.
 *
 * KPopupMenu is a class for menus with standard title items and keyboard
 * accessibility for popups with many options and/or varying options. It acts
 * identically to QPopupMenu, with the addition of insertTitle(),
 * changeTitle(), setKeyboardShortcutsEnabled() and
 * setKeyboardShortcutsExecute() methods.
 *
 * The titles support a text string, an icon, plus user defined gradients,
 * colors, and background pixmaps.
 *
 * The keyboard search algorithm is incremental with additional underlining
 * for user feedback.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 * @author Hamish Rodda <rodda@kde.org>
 */
class KDEUI_EXPORT KPopupMenu : public QPopupMenu {
    Q_OBJECT
public:
    /**
     * Constructs a KPopupMenu.
     */
    KPopupMenu(QWidget *parent=0, const char *name=0);

    /**
     * Destructs the object
     */
    ~KPopupMenu();

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
     * @deprecated
     * Obsolete method provided for backwards compatibility only. Use the
     * normal constructor and insertTitle instead.
     */
    KPopupMenu(const QString &title, QWidget *parent=0, const char *name=0) KDE_DEPRECATED;

    /**
     * @deprecated
     * Obsolete method provided for backwards compatibility only. Use
     * insertTitle and changeTitle instead.
     */
    void setTitle(const QString &title) KDE_DEPRECATED;

    /**
     * Returns the context menu associated with this menu
     * @since 3.2
     */
    QPopupMenu* contextMenu();

    /**
     * Returns the context menu associated with this menu
     * @since 3.2
     */
    const QPopupMenu* contextMenu() const;

    /**
     * Hides the context menu if shown
     * @since 3.2
     */
    void hideContextMenu();

    /**
     * Returns the KPopupMenu associated with the current context menu
     * @since 3.2
     */
    static KPopupMenu* contextMenuFocus();

    /**
     * returns the ID of the menuitem associated with the current context menu
     * @since 3.2
     */
    static int contextMenuFocusItem();

    /**
     * Reimplemented for internal purposes
     * @since 3.4
     */
    virtual void activateItemAt(int index);
    /**
     * Return the state of the mouse button and keyboard modifiers
     * when the last menuitem was activated.
     * @since 3.4
     */
    Qt::ButtonState state() const;

signals:
    /**
     * connect to this signal to be notified when a context menu is about to be shown
     * @param menu The menu that the context menu is about to be shown for
     * @param menuItem The menu item that the context menu is currently on
     * @param ctxMenu The context menu itself
     * @since 3.2
     */
    void aboutToShowContextMenu(KPopupMenu* menu, int menuItem, QPopupMenu* ctxMenu);

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
    void itemHighlighted(int whichItem);
    void showCtxMenu(QPoint pos);
    void ctxMenuHiding();
    void ctxMenuHideShowingMenu();

private:
    class KPopupMenuPrivate;
    KPopupMenuPrivate *d;
};

#endif
