/* This file is part of the KDE libraries
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

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

#ifndef __ktexteditor_menuinterface_h__
#define __ktexteditor_menuinterface_h__

#include <kdelibs_export.h>

class QMenu;

namespace KTextEditor
{

/**
*  This is an interface to provide custom popup menus for a View.
*/
class KTEXTEDITOR_EXPORT MenuInterface
{
  public:
    virtual ~MenuInterface () {}

  //
  // normal methodes
  //
  public:
     /**
      Install a Popup Menu. The Popup Menu will be activated on
      a right mouse button press event.
    */
    virtual void installMenu (QMenu *rmb_Menu) = 0;
};

KTEXTEDITOR_EXPORT MenuInterface *menuInterface (class View *view);

}

#endif
