/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
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

#ifndef KPUSHBUTTON_H
#define KPUSHBUTTON_H

#include <qpoint.h>
#include <qpushbutton.h>
class QDragObject;

/**
 * This is nothing but a QPushButton with drag-support. You have to call
 * @ref setDragEnabled( true ) and override the virtual method 
 * @ref dragObject() to specify the QDragObject to be used.
 *
 * @short A QPushButton with drag-support
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KPushButton : public QPushButton
{
    Q_OBJECT

public:
    /**
     * Default constructor.
     */
    KPushButton( QWidget *parent, const char *name=0 );

    /**
     * Constructor, that sets the button-text to @p text
     */
    KPushButton( const QString &text, QWidget *parent, const char *name=0);

    /**
     * Constructor, that sets an icon and the button-text to @p text
     */
    KPushButton( const QIconSet &icon, const QString &text,
		     QWidget *parent, const char *name=0 );

    /**
     * Destroys the button
     */
    ~KPushButton();

    /**
     * Enables/disables drag-support. Default is enabled.
     */
    void setDragEnabled( bool enable );

    /**
     * @returns if drag support is enabled or not.
     */
    bool isDragEnabled() const { return m_dragEnabled; }


protected:
    /**
     * Reimplement this and return the QDragObject that should be used
     * for the drag.
     * Default implementation returns 0L, so that no drag is initiated.
     */
    virtual QDragObject * dragObject();

    /**
     * Reimplemented to add drag-support
     */
    virtual void mousePressEvent( QMouseEvent * );
    /**
     * Reimplemented to add drag-support
     */
    virtual void mouseMoveEvent( QMouseEvent * );

    /**
     * Starts a drag (dragCopy() by default) using @ref dragObject()
     */
    virtual void startDrag();

private:
    bool m_dragEnabled;
    QPoint startPos;

private:
    class KPushButtonPrivate;
    KPushButtonPrivate *d;

};

#endif // KPUSHBUTTON_H
