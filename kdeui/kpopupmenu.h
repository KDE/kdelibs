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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef _KPOPUP_H
#define _KPOPUP_H "$Id$"

#define INCLUDE_MENUITEM_DEF

#include <qpopupmenu.h>
#include <kpixmapeffect.h>

/**
 * Title widget for use in @ref KPopupMenu.
 *
 * You usually don't have to create this manually since
 * @ref KPopupMenu::insertTitle will do it for you, but it is allowed if
 * you wish to customize it's look.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 * @short KPopupMenu title widget.
 */
class KPopupTitle : public QWidget
{
public:
    /**
     * Create a title widget with the user specified gradient, pixmap,
     * and colors.
     */
    KPopupTitle(QWidget *parent=0, const char *name=0);
    /**
     * Create a title widget with the specified gradient and colors.
     */
    KPopupTitle(KPixmapEffect::GradientType gradient, const QColor &color,
                const QColor &textColor, QWidget *parent=0,
                const char *name=0);
    /**
     * Create a title widget with the specified pixmap and colors.
     */
    KPopupTitle(const KPixmap &background, const QColor &color,
                const QColor &textColor, QWidget *parent=0,
                const char *name=0);
    /**
     * Set the title string and optional icon for the title widget.
     *
     * You will
     * want to call this before inserting into a menu.
     */
    void setTitle(const QString &text, const QPixmap *icon=NULL);
    /**
     * Retrieve the current title.
     */
    QString title() const { return(titleStr); }
    /**
     * Retrieve the current icon.
     */
    QPixmap icon() const { return(miniicon); }
    QSize sizeHint() const;
protected:
    void paintEvent(QPaintEvent *ev);

    KPixmapEffect::GradientType grType;
    QString titleStr;
    KPixmap fill;
    QPixmap miniicon;
    QColor fgColor, bgColor, grHigh, grLow;
    bool useGradient;

    class KPopupTitlePrivate;
    KPopupTitlePrivate *d;
};

/**
 * KPopupMenu is a class for menus with standard title items. It acts
 * identically to QPopupMenu, with the addition of insertTitle() and
 * changeTitle() methods.
 *
 * The titles support a text string, an icon, plus user defined gradients,
 * colors, and background pixmaps.
 *
 * @short A menu with title items.
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class KPopupMenu : public QPopupMenu {
    Q_OBJECT
public:
    /**
     * Creates a new KPopupMenu.
     */
    KPopupMenu(QWidget *parent=0, const char *name=0);
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
     * Obsolete method provided for backwards compatibility only. Use the
     * normal constructor and insertTitle instead.
     */
    KPopupMenu(const QString &title, QWidget *parent=0, const char *name=0);
    /**
     * Obsolete method provided for backwards compatibility only. Use
     * insertTitle and changeTitle instead.
     */
    void setTitle(const QString &title);

private:
    // For backwards compatibility
    QString lastTitle;

    class KPopupMenuPrivate;
    KPopupMenuPrivate *d;
};

#endif
