/* This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)
              (C) 1997 Matthias Ettrich (ettrich@kde.org)
 
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
#ifndef KBUTTON_H
#define KBUTTON_H

#include <qpixmap.h>
#include <qbutton.h>

/**
* Provides active-raise/lower buttons.
* @version $Id$
*/
class KButton : public QButton
{
    Q_OBJECT
public:
    KButton( QWidget *_parent = 0L, const char *name = 0L );
    ~KButton();

protected:
    virtual void leaveEvent( QEvent *_ev );
    virtual void enterEvent( QEvent *_ev );
        
    virtual void drawButton( QPainter *_painter );
    virtual void drawButtonLabel( QPainter *_painter );

    void paint( QPainter *_painter );

 private:    
    bool raised;    
};

#endif
