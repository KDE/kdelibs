/*  This file is part of the KDE Libraries
    Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)

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

#ifndef __KDIRECTIONBUTTON_H__
#define __KDIRECTIONBUTTON_H__

#include <qdrawutil.h>

class QButton;
class QPainter;
class QWidget;

/**
* KDirectionButton is a helper class for KTabBar and KWizard. It provides the buttons
* used to scroll the tab bar and to change pages in KWizard.
* @short KDirectionButton
* @author Thomas Tanghus <tanghus@earthling.net>
* @version 0.1
*/
class KDirectionButton : public QButton
{
    Q_OBJECT
public:
    KDirectionButton( QWidget * parent = 0, const char * name = 0 );
    KDirectionButton( ArrowType d, QWidget * parent = 0, const char * name = 0 );
   ~KDirectionButton();

    void setDirection( ArrowType d) { direct = d; };
    ArrowType direction( ) { return direct; };

protected:
   virtual void drawButton(QPainter *);

   ArrowType direct;
};

/**
* KTabButton is a helper class for KTabBar. It provides the buttons
* used to scroll the tab bar.
* @short KTabButton
* @author Thomas Tanghus <tanghus@earthling.net>
* @version 0.1
*/
class KTabButton : public KDirectionButton
{
    Q_OBJECT
public:
    KTabButton( QWidget * parent = 0, const char * name = 0 );
    KTabButton( ArrowType d, QWidget * parent = 0, const char * name = 0 );
   ~KTabButton() { };

protected:
   virtual void drawButton(QPainter *);
};

#endif // __KDIRECTIONBUTTON_H__


