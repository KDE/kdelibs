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

/**
* This widget can be used to display or allow user selection of a colour.
*
* @see KColorDialog
*
* @short A pushbutton to display or allow user selection of a colour.
* @version $Id$
*/
class KColorButton : public QPushButton
{
	Q_OBJECT
public:
	/**
	* Constructor. Create a KColorButton.
	*/
	KColorButton( QWidget *parent, const char *name = 0L );
	/**
	* Constructor. Create a KColorButton.
	* @param c	The initial colour of the button.
	*/
	KColorButton( const QColor &c, QWidget *parent, const char *name = 0L );
	/**
	* Destructor.
	*/
	virtual ~KColorButton() {}

	/**
	* The current colour.
	* @return The current colour.
	*/
	const QColor color() const
		{	return col; }
	/**
	* Set the current colour.
	*
	* @param c	The colour to set.
	*/
	void setColor( const QColor &c );

signals:
	/**
	* This signal will be emitted when the colour of the widget
	* is changed, either with @ref #setColor or via user selection.
	*/
	void changed( const QColor &newColor );

protected slots:

protected:
	/**
	*/
	virtual void drawButtonLabel( QPainter *p );

	/**
	 * For XDnd
	 */
	virtual void dragEnterEvent( QDragEnterEvent *);
        virtual void dropEvent( QDropEvent *);
        virtual void mousePressEvent( QMouseEvent *e );
        virtual void mouseMoveEvent( QMouseEvent *e);
        virtual void mouseReleaseEvent ( QMouseEvent *e );
private:
        QColor col;
        QPoint mPos;
        bool inMouse, dragged;
};

#endif

