/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KTEXTEDIT_H
#define KTEXTEDIT_H

#include <qtextedit.h>

class KTextEdit : public QTextEdit
{
    Q_OBJECT
    
public:
    KTextEdit( const QString& text,
               const QString& context = QString::null, 
               QWidget * parent = 0, const char *name = 0 );
    KTextEdit( QWidget *parent = 0L, const char *name = 0 );
    ~KTextEdit();
    
    virtual void setReadOnly (bool readOnly)

protected:
    virtual void keyPressEvent( QKeyEvent * );
    virtual void contentsWheelEvent( QWheelEvent * );

    virtual void deleteWordBack();
    virtual void deleteWordForward();
    
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KTextEditPrivate;
    KTextEditPrivate *d;

};

#endif // KTEXTEDIT_H
