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
 * 03/08/2000 Mario Weilguni <mweilguni@kde.org>
 * Removed all those long outdated Motif stuff
 * Improved and clarified some if conditions (easier to understand)
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

#include "kbuttonbox.moc"
#include <qpushbutton.h>
#include <qptrlist.h>
#include <assert.h>

#define minButtonWidth 50

class KButtonBox::Item {
public:
  QPushButton *button;
  bool noexpand;
  unsigned short stretch;
  unsigned short actual_size;
};

template class QPtrList<KButtonBox::Item>;

class KButtonBox::PrivateData {
public:
  unsigned short border;
  unsigned short autoborder;
  unsigned short orientation;
  bool activated;
  QPtrList<KButtonBox::Item> buttons;
};

KButtonBox::KButtonBox(QWidget *parent, Orientation _orientation,
		       int border, int autoborder)
  :  QWidget(parent)
{
  data = new PrivateData;
  assert(data != 0);

  data->orientation = _orientation;
  data->border = border;
  data->autoborder = autoborder < 0 ? border : autoborder;
  data->buttons.setAutoDelete(TRUE);
}

KButtonBox::~KButtonBox() {
  delete data;
}

QPushButton *KButtonBox::addButton(const QString& text, bool noexpand) {
  Item *item = new Item;

  item->button = new QPushButton(text, this);
  item->noexpand  = noexpand;
  data->buttons.append(item);
  item->button->adjustSize();

  return item->button;
}

  QPushButton *
KButtonBox::addButton(
  const QString & text,
  QObject *       receiver,
  const char *    slot,
  bool            noexpand
)
{
  QPushButton * pb = addButton(text, noexpand);

  if ((0 != receiver) && (0 != slot))
    QObject::connect(pb, SIGNAL(clicked()), receiver, slot);

  return pb;
}


void KButtonBox::addStretch(int scale) {
  if(scale > 0) {
    Item *item = new Item;
    item->button = 0;
    item->noexpand  = FALSE;
    item->stretch = scale;
    data->buttons.append(item);
  }
}

void KButtonBox::layout() {
  // resize all buttons
  QSize bs = bestButtonSize();

  for(unsigned int i = 0; i < data->buttons.count(); i++) {
    Item *item = data->buttons.at(i);
    QPushButton *b = item->button;
    if(b != 0) {
      if(item->noexpand)
	b->setFixedSize(buttonSizeHint(b));
      else
	b->setFixedSize(bs);
    }
  }

  setMinimumSize(sizeHint());
}

void KButtonBox::placeButtons() {
  unsigned int i;

  if(data->orientation == Horizontal) {
    // calculate free size and stretches
    int fs = width() - 2 * data->border;
    int stretch = 0;
    for(i = 0; i < data->buttons.count(); i++) {
      Item *item = data->buttons.at(i);
      if(item->button != 0) {
	fs -= item->button->width();

	// Last button?
	if(i != data->buttons.count() - 1)
	  fs -= data->autoborder;
      } else
	stretch +=item->stretch;
    }

    // distribute buttons
    int x_pos = data->border;
    for(i = 0; i < data->buttons.count(); i++) {
      Item *item = data->buttons.at(i);
      if(item->button != 0) {
	QPushButton *b = item->button;
	b->move(x_pos, (height() - b->height()) / 2);

	x_pos += b->width() + data->autoborder;
      } else
	x_pos += (int)((((double)fs) * item->stretch) / stretch);
    }
  } else { // VERTICAL
    // calcualte free size and stretches
    int fs = height() - 2 * data->border;
    int stretch = 0;
    for(i = 0; i < data->buttons.count(); i++) {
      Item *item = data->buttons.at(i);
      if(item->button != 0)
	fs -= item->button->height() + data->autoborder;
      else
	stretch +=item->stretch;
    }

    // distribute buttons
    int y_pos = data->border;
    for(i = 0; i < data->buttons.count(); i++) {
      Item *item = data->buttons.at(i);
      if(item->button != 0) {
	QPushButton *b = item->button;
	b->move((width() - b->width()) / 2, y_pos);

	y_pos += b->height() + data->autoborder;
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
  unsigned int i;

  // calculate optimal size
  for(i = 0; i < data->buttons.count(); i++) {
    KButtonBox *that = (KButtonBox*)this; // to remove the const ;(
    Item *item = that->data->buttons.at(i);
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
  unsigned int i, dw;

  if(data->buttons.count() == 0)
    return QSize(0, 0);
  else {
    dw = 2 * data->border;

    QSize bs = bestButtonSize();
    for(i = 0; i < data->buttons.count(); i++) {
      KButtonBox *that = (KButtonBox*)this;
      Item *item = that->data->buttons.at(i);
      QPushButton *b = item->button;
      if(b != 0) {
	QSize s;
	if(item->noexpand)
	  s = that->buttonSizeHint(b);
	else
	  s = bs;

	if(data->orientation == Horizontal)
	  dw += s.width();
	else
	  dw += s.height();

	if( i != data->buttons.count() - 1 )
	  dw += data->autoborder;
      }
    }

    if(data->orientation == Horizontal)
	return QSize(dw, bs.height() + 2 * data->border);
    else
	return QSize(bs.width() + 2 * data->border, dw);
  }
}

QSizePolicy KButtonBox::sizePolicy() const
{
    return data->orientation == Horizontal?
        QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) :
        QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum );
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

