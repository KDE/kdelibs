/* This file is part of the KDE libraries
   Copyright (c) 1999 Daniel Duley <mosfet@kde.org>

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
#ifndef __KSTYLE_H
#define __KSTYLE_H

#include <qplatinumstyle.h>
#include <qfont.h>
#include <qpalette.h>
#include <qpixmap.h>

class QMenuItem;
class QPixmap;
class KStylePrivate;

/**
 * Extends the @ref QStyle class with virtual methods to draw KDE widgets.
 *
 * To create a new @ref KStyle, reimplement the virtual functions which draw
 *  the GUI elements.
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class KStyle : public QPlatinumStyle
{
    Q_OBJECT
public:
    enum KToolButtonType{Icon=0, IconTextRight, Text, IconTextBottom};
    KStyle() : QPlatinumStyle(){;}
    /**
     * Draw a toolbar (without buttons, etc.)
     *
     * @p x, @y, @w, and @h tell in which rectangle to draw the toolbar.
     * @param floating @true of this is a floating toolbar, @false if it
     *  is part of the application window.
     **/
    virtual void drawKToolBar(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, bool floating = false);
    /**
     * Draw a menubar/toolbar handle.
     *
     * @param horizontal If @true, the menubar/toolbar is oriented horizontally.
     * @param fill The @ref QBrush to fill the handle with.
     **/
    virtual void drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool horizontal=false,
                                QBrush *fill=NULL);
    /**
     * Draw a toolbar button.
     **/
    virtual void drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, bool sunken=false,
                                    bool raised = true, bool enabled = true,
                                    bool popup = false,
                                    KToolButtonType icontext = Icon,
                                    const QString& btext=QString::null,
                                    const QPixmap *icon=NULL,
                                    QFont *font=NULL);
    /**
     * Draw a menubar (without menus).
     *
     * @param fill The @ref QBrush to fill the menubar with.
     **/
    virtual void drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, QBrush *fill=NULL);
    /**
     * Draw a menu item (ex, "&File").
     **/
    virtual void drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, bool active,
                               QMenuItem *item, QBrush *fill=NULL);
    /**
     * Draw one block on a progress bar.
     **/
    virtual void drawKProgressBlock(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QBrush *fill);
    /**
     * Retrieve the color used to the background of a progress bar.
     *
     * This isn't a drawXXX method due to @ref KProgress implementation.
     **/
    virtual void getKProgressBackground(const QColorGroup &g, QBrush &bg);

    /**
     * Internal method for use in the new OpenParts implementation. You should
     * not override this but drawKToolBar instead.
     * @internal
     */
    virtual void drawOPToolBar(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, QBrush *fill=NULL);

private:
    KStylePrivate *d;
};

#endif
    
    
