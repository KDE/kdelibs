/* This file is part of the KDE libraries
    Copyright (C) 1997 Mario Weilguni (mweilguni@sime.com)

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
/* 
 * KButtonBox class
 *
 * A container widget for buttons. Uses Qt layout control to place the
 * buttons, can handle both vertical and horizontal button placement.
 * The default border is now 0 (easier to deal with layouts). The space
 * between buttons is now more Motif compliant
 */

#ifndef __KBUTTONBOX__H__
#define __KBUTTONBOX__H__

#include <qwidget.h>
#include <qpushbt.h>
#include <qlist.h>

/**
  * This class is used internally.
  */
class KButtonBoxItem {
public:
  QPushButton *button;
  bool noexpand;
  int stretch;
  int actual_size;
};


class KButtonBox : public QWidget {
  Q_OBJECT
public:
  enum { VERTICAL = 1, HORIZONTAL = 2 };

  /**
    * Creates an empty container for buttons. If _orientation is 
    * KButtonBox::VERTICAL, the buttons inserted with @see addButton
    * are layouted from top to bottom, otherwise they are layouted 
    * from left to right.
    */
  KButtonBox(QWidget *parent, int _orientation = HORIZONTAL, 
	     int border = 0, int _autoborder = 6);

  /**
    * The destructor is needed, otherwise gcc 2.7.2.1 may report an 
    * internal compiler error. It does nothing.
    */
  ~KButtonBox();

  /**
    * @return the minimum size needed to fit all buttons. This size is
    * calculated by the with/height of all buttons plus border/autoborder
    */
  virtual QSize sizeHint() const;

  virtual void resizeEvent(QResizeEvent *);

  /**
    * adds a new @see QPushButton and @return a pointer to the newly 
    * created button. If noexpand is FALSE, the width of the button is
    * adjusted to fit the other buttons (the maximum of all buttons is
    * taken). If noexpand is TRUE, the width of this button will be
    * set to the minimum width needed for the given text).
    */
  QPushButton *addButton(const char *text, bool noexpand = FALSE);

  /**
    * This adds a stretch to the buttonbox. @see QBoxLayout for details.
    * Can be used to separate buttons (i.e. if you add the buttons "Ok",
    * "Cancel", add a stretch and then add the button "Help", "Ok" and
    * "Cancel" will be left-aligned (or top-aligned for vertical) while
    * "Help" will be right-aligned (or bottom-aligned for vertical).
    */
  void addStretch(int scale = 1);

  /**
    * This function must be called ONCE after all buttons have been
    * inserted. It will start layout control.
    */
  void layout();

protected:
  /**
    * @return the best size for a button. Checks all buttons and takes
    * the maximum width/height.
    */
  QSize bestButtonSize() const;
  void  placeButtons();
  QSize buttonSizeHint(QPushButton *) const;

protected:
  int _border, _autoborder;
  int orientation;
  bool activated;
  QList<KButtonBoxItem> buttons;
};

#endif
