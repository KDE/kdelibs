/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __COLBTN_H__
#define __COLBTN_H__

#include <qpushbutton.h>

#include <kdelibs_export.h>

class KColorButtonPrivate;
/**
* @short A pushbutton to display or allow user selection of a color.
*
* This widget can be used to display or allow user selection of a color.
*
* @see KColorDialog
*
* \image html kcolorbutton.png "KDE Color Button"
*
* @version $Id$
*/
class KDEUI_EXPORT KColorButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY( QColor color READ color WRITE setColor )

public:
    /**
     * Creates a color button.
     */
    KColorButton( QWidget *parent, const char *name = 0L );

    /**
     * Creates a color button with an initial color @p c.
     */
    KColorButton( const QColor &c, QWidget *parent, const char *name = 0L );
    /// @since 3.1
    KColorButton( const QColor &c, const QColor &defaultColor, QWidget *parent,
                  const char *name=0L );

    virtual ~KColorButton();

    /**
     * Returns the currently chosen color.
     */
    QColor color() const
        { return col; }

    /**
     * Sets the current color to @p c.
     */
     void setColor( const QColor &c );

    QSize sizeHint() const;

signals:
    /**
     * Emitted when the color of the widget
     * is changed, either with setColor() or via user selection.
     */
    void changed( const QColor &newColor );

protected slots:
    void chooseColor();

protected:
    virtual void drawButtonLabel( QPainter *p );
    virtual void dragEnterEvent( QDragEnterEvent *);
    virtual void dropEvent( QDropEvent *);
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e);
    virtual void keyPressEvent( QKeyEvent *e );
private:
    QColor col;
    QPoint mPos;
    bool dragFlag;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KColorButtonPrivate;
    KColorButtonPrivate *d;
};

#endif

