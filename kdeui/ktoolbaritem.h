/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)

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

// $Id$
// $Log$
// Revision 1.1  1999/09/21 11:03:54  waba
// WABA: Clean up interface
//

#ifndef _KTOOLBARITEM_H
#define _KTOOLBARITEM_H

#include <qwidget.h>

class KToolBar;

typedef QWidget Item;

enum itemType {
    ITEM_LINED = 0,
    ITEM_BUTTON = 1,
    ITEM_COMBO = 2,
    ITEM_FRAME = 3,
    ITEM_TOGGLE = 4,
    ITEM_ANYWIDGET=5
};

/**
 * A toolbar item. Used internally by KToolBar, use KToolBar methods
 * instead.
 * @internal
 * */
class KToolBarItem
{
public:
  KToolBarItem (Item *_item, itemType _type, int _id,
                bool _myItem=true);
  ~KToolBarItem ();

  void resize (int w, int h) { item->resize(w, h); };
  void move(int x, int y) { item->move(x, y); };
  void show () { item->show(); };
  void hide () { item->hide(); };
  void setEnabled (bool enable) { item->setEnabled(enable); };
  bool isEnabled () { return item->isEnabled(); };
  int ID() { return id; };
  bool isRight () const { return right; };
  void alignRight  (bool flag) { right = flag; };
  void autoSize (bool flag) { autoSized = flag; };
  bool isAuto () const { return autoSized; };
  int width() const { return item->width(); };
  int height() const { return item->height(); };
  int x() const { return item->x(); };
  int y() const { return item->y(); };
  int winId () { return item->winId(); };

  Item *getItem() { return item; };

private:
  int id;
  bool right;
  bool autoSized;
  Item *item;
  itemType type;
  bool myItem;
};

/** 
* List of @ref KToolBarItem.
* @internal
*/
class KToolBarItemList : public QList<KToolBarItem>
{
public:
   KToolBarItemList() { setAutoDelete(true); }
   ~KToolBarItemList() { }
};  

#endif
