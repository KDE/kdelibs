/* This file is part of the KDE libraries
   Copyright (C) 2005 David Faure <faure@kde.org>

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

#ifndef KHBOX_H
#define KHBOX_H

#include <qframe.h>
#include <kdelibs_export.h>

/**
 * A container widget which arranges its children horizontally.
 * When using a KHBox you don't need to create a layout nor to add the child widgets to it.
 */
class KDEUI_EXPORT KHBox : public QFrame
{
    Q_OBJECT
public:
    KHBox( QWidget* parentWidget );

    /**
     * Sets the spacing between the child widgets to @p space
     */
    void setSpacing( int space );
    /*
     * Sets the stretch factor of widget @p w to @p stretch.
     */
    void setStretchFactor( QWidget* w, int stretch );

    virtual QSize sizeHint() const;

    void setMargin(int margin);
protected:
    /*
     * @internal
     */
    KHBox( bool vertical, QWidget* parentWidget );

    virtual bool event( QEvent* ev );

private:
    Q_DISABLE_COPY(KHBox)
};


#endif
