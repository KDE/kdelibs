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
*
 * HISTORY
 *
 * 11/13/98 Reginald Stadlbauer <reggie@kde.org>
 * Now in Qt 1.4x motif default buttons have no extra width/height anymore.
 * So the KButtonBox doesn't add this width/height to default buttons anymore
 * which makes the buttons look better.
 *
 * 01/17/98  Mario Weilguni <mweilguni@sime.com>
 * Fixed a bug in sizeHint()
 * Improved the handling of Motif default buttons
 *
 * 01/09/98  Mario Weilguni <mweilguni@sime.com>
 * The last button was to far right away from the right/bottom border.
 * Fixed this. Removed old code. Buttons get now a minimum width.
 * Programmer may now override minimum width and height of a button.
 *
 */

#include "kbuttonbox.h"

// taken from Qt source
// Reggie: In Qt 1.4x we don't have additional width/height!
const int extraMotifWidth = 0;
const int extraMotifHeight = 0;

const int minButtonWidth = 50;

KButtonBox::KButtonBox(QWidget *parent, int _orientation, 
		       int border, int autoborder) 
  :  QWidget(parent) 
{
  orientation = _orientation;
  _border = border;
  if(autoborder < 0)
    _autoborder = border;
  else
    _autoborder = autoborder;

  buttons.setAutoDelete(TRUE);
}

KButtonBox::~KButtonBox() {
  // an empty destructor is needed for g++ if a class uses
  // a qlist, qarray or similar, otherwise it will not
  // compile
}

QPushButton *KButtonBox::addButton(const QString& text, bool noexpand) {
  KButtonBoxItem *item = new KButtonBoxItem;

  item->button = new QPushButton(text, this);
  item->noexpand  = noexpand;
  buttons.append(item);
  item->button->adjustSize();
  
  return item->button;
}

void KButtonBox::addStretch(int scale) {
  if(scale > 0) {
    KButtonBoxItem *item = new KButtonBoxItem;
    item->button = 0;
    item->noexpand  = FALSE;
    item->stretch = scale;  
    buttons.append(item);
  }
}

void KButtonBox::layout() {
  // resize all buttons
  QSize bs = bestButtonSize();

  for(unsigned i = 0; i < buttons.count(); i++) {
    KButtonBoxItem *item = buttons.at(i);
    QPushButton *b = item->button;
    if(b != 0) {
      if(!item->noexpand) {
	if(b->style() == MotifStyle && b->isDefault()) {
	  QSize s = bs;
	  s.setWidth(bs.width() + extraMotifWidth);
	  s.setHeight(bs.height() + extraMotifHeight);
	  b->setFixedSize(s);
	  b->setDefault(TRUE);
	} else {
	  b->setFixedSize(bs);
	}
      } else
	b->setFixedSize(buttonSizeHint(b));
    }
  }  

  setMinimumSize(sizeHint());
}

void KButtonBox::placeButtons() {
  unsigned i;

  if(orientation == HORIZONTAL) {
    // calcualte free size and stretches
    int fs = width() - 2 * _border;
    int stretch = 0;
    for(i = 0; i < buttons.count(); i++) {
      KButtonBoxItem *item = buttons.at(i);
      if(item->button != 0) {
	if(i == buttons.count() - 1)
	  fs -= item->button->width();
	else
	  fs -= item->button->width() + _autoborder;
      } else
	stretch +=item->stretch;
    }

    // distribute buttons
    int x_pos = _border;
    for(i = 0; i < buttons.count(); i++) {
      KButtonBoxItem *item = buttons.at(i);
      if(item->button != 0) {
	QPushButton *b = item->button;
	if(b->style() == MotifStyle && b->isDefault()) {
	  b->move(x_pos + extraMotifWidth/2, 
		  (height() - b->height()) / 2 + extraMotifHeight/2);
	  if(_autoborder < extraMotifWidth/2)
	    x_pos += extraMotifWidth;
	} else
	  b->move(x_pos, (height() - b->height()) / 2);
      
	x_pos += b->width() + _autoborder;
      } else
	x_pos += (int)((((double)fs) * item->stretch) / stretch);
    }
  } else { // VERTICAL
    // calcualte free size and stretches
    int fs = height() - 2 * _border;
    int stretch = 0;
    for(i = 0; i < buttons.count(); i++) {
      KButtonBoxItem *item = buttons.at(i);
      if(item->button != 0) 
	fs -= item->button->height() + _autoborder;
      else
	stretch +=item->stretch;
    }

    // distribute buttons
    int y_pos = _border;
    for(i = 0; i < buttons.count(); i++) {
      KButtonBoxItem *item = buttons.at(i);
      if(item->button != 0) {
	QPushButton *b = item->button;
	if(b->style() == MotifStyle && b->isDefault()) {
	  b->move((width() - b->width()) / 2 + extraMotifWidth/2,
		  y_pos + extraMotifHeight/2);
	  if(_autoborder < extraMotifHeight/2)
	    y_pos += extraMotifHeight;
	} else
	  b->move((width() - b->width()) / 2,
		  y_pos);
      
	y_pos += b->height() + _autoborder;
      } else
	y_pos += (int)((((double)fs) * item->stretch) / stretch);
    }
  }
}

void KButtonBox::resizeEvent(QResizeEvent *) {
  placeButtons();
}

QSize KButtonBox::bestButtonSize() const {
  QSize s(0, 0);
  unsigned i;

  // calculate optimal size
  for(i = 0; i < buttons.count(); i++) {
    KButtonBox *that = (KButtonBox*)this; // to remove the const ;(
    KButtonBoxItem *item = that->buttons.at(i);
    QPushButton *b = item->button;
 
    if(b != 0 && !item->noexpand) {      
      QSize bs = buttonSizeHint(b);

      if(bs.width() > s.width())
	s.setWidth(bs.width());
      if(bs.height() > s.height())
	s.setHeight(bs.height());      
    }
  }

  return s;
}

QSize KButtonBox::sizeHint() const {
  unsigned i, dw;
  bool hasMotifDefault = FALSE;

  if(buttons.count() == 0)
    return QSize(0, 0);
  else {
    dw = 2 * _border;

    QSize bs = bestButtonSize();
    for(i = 0; i < buttons.count(); i++) {
      KButtonBox *that = (KButtonBox*)this;
      KButtonBoxItem *item = that->buttons.at(i);
      QPushButton *b = item->button;
      if(b != 0) {
	hasMotifDefault |= (style() == MotifStyle) && (b->isDefault());

	QSize s;
	if(item->noexpand)
	  s = that->buttonSizeHint(b);
	else
	  s = bs;
	
	if(orientation == HORIZONTAL)
	  dw += s.width();
	else
	  dw += s.height();

	if( i != buttons.count() - 1 )
	  dw += _autoborder;
      }
    }

    if(orientation == HORIZONTAL) {
      if(style() == MotifStyle && hasMotifDefault) 	
	return QSize(dw + extraMotifWidth, 
		     bs.height() + 2 * _border + extraMotifHeight);
      else
	return QSize(dw, bs.height() + 2 * _border);
    } else {
      if(style() == MotifStyle)
	return QSize(bs.width() + 2 * _border + extraMotifWidth, 
		     dw + extraMotifHeight);
      else
	return QSize(bs.width() + 2 * _border, dw);
    }
  }  
}

/*
 * Returns the best size for a button. If a button is less than 
 * minButtonWidth pixels wide, return minButtonWidth pixels 
 * as minimum width
 */
QSize KButtonBox::buttonSizeHint(QPushButton *b) const {
  QSize s = b->sizeHint();  
  QSize ms = b->minimumSize();
  if(s.width() < minButtonWidth)
    s.setWidth(minButtonWidth);

  // allows the programmer to override the settings
  if(ms.width() > s.width())
    s.setWidth(ms.width());
  if(ms.height() > s.height())
    s.setHeight(ms.height());
  
  return s;
}

#include "kbuttonbox.moc"
