/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998, 1999, 2000  Sven Radej (radej@kde.org)
    Copyright (C) 1997, 1998, 1999, 2000 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999, 2000 Daniel "Mosfet" Duley (mosfet@kde.org)

    
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

//$Id$

#ifndef _KMENUBAR_H
#define _KMENUBAR_H

#include <qmenubar.h>
#include <kglobal.h>

/**
 * KDE Style-able menubar.
 *
 * This is required since @ref QMenuBar is currently not handled by
 * @ref QStyle.
 *
 * @author Daniel "Mosfet" Duley.
 * @version $Id$
*/

class KMenuBar : public QMenuBar
{
    Q_OBJECT
    
public:

    KMenuBar (QWidget *parent=0, const char *name=0);
    ~KMenuBar ();

protected slots:
    void slotReadConfig();

protected:
    void drawContents(QPainter *p);
    void enterEvent(QEvent *ev);
    void leaveEvent(QEvent *ev);
    bool eventFilter(QObject *, QEvent *);

    bool mouseActive;

    class KMenuBarPrivate;
    KMenuBarPrivate *d;
};

#endif
