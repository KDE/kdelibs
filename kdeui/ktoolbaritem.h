/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
              (C) 2000 Kurt Granroth (granroth@kde.org)

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
// $Id$
#ifndef _KTOOLBARITEM_H
#define _KTOOLBARITEM_H

#include <qwidget.h>
#include <qlist.h>

class KToolBar;
class KToolBarItemPrivate;

typedef QWidget Item;


/**
 * A toolbar item. Used internally by KToolBar, use KToolBar methods
 * instead.
 * @internal
 * */
class KToolBarItem
{
public:
  enum ItemType
  {
    Lined = 0, Button, Combo, Frame, Toggle, AnyWidget, Separator
  };

  KToolBarItem (Item *_item, ItemType _type, int _id,
                bool _myItem=true);
  ~KToolBarItem ();

  ItemType itemType() const { return type; }

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
  ItemType type;
  bool myItem;

  KToolBarItemPrivate *d;
};

#endif
