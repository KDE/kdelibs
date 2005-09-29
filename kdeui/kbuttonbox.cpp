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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

/*
 * KButtonBox class
 *
 * A container widget for buttons. Uses Qt layout control to place the
 * buttons, can handle both vertical and horizontal button placement.
*
 * HISTORY
 *
 * 05/11/2004 Andrew Coles <andrew_coles@yahoo.co.uk>
 * Now uses QPtrListIterators instead of indexing through data->buttons
 * Item.button and data are now const pointers, set in the relevant constructors
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
#include <kglobalsettings.h>
#include <kguiitem.h>
#include <kpushbutton.h>
#include <qptrlist.h>
#include <assert.h>

#define minButtonWidth 50

class KButtonBox::Item {
public:
  KPushButton* const button;
  bool noexpand;
  unsigned short stretch;
  unsigned short actual_size;

  Item(KPushButton* const _button) : button(_button) {};
};

template class QPtrList<KButtonBox::Item>;

class KButtonBoxPrivate {
public:
  unsigned short border;
  unsigned short autoborder;
  unsigned short orientation;
  bool activated;
  QPtrList<KButtonBox::Item> buttons;
};

KButtonBox::KButtonBox(QWidget *parent, Orientation _orientation,
		       int border, int autoborder)
  :  QWidget(parent), data(new KButtonBoxPrivate)
{
  assert(data);

  data->orientation = _orientation;
  data->border = border;
  data->autoborder = autoborder < 0 ? border : autoborder;
  data->buttons.setAutoDelete(true);
}

KButtonBox::~KButtonBox() {
  delete data;
}

QPushButton *KButtonBox::addButton(const QString& text, bool noexpand) {
  Item* const item = new Item(new KPushButton(text, this));

  item->noexpand  = noexpand;
  data->buttons.append(item);
  item->button->adjustSize();

  this->updateGeometry();

  return item->button;
}

QPushButton *KButtonBox::addButton(const KGuiItem& guiitem, bool noexpand) {
  Item* const item = new Item(new KPushButton(guiitem, this));

  item->noexpand  = noexpand;
  data->buttons.append(item);
  item->button->adjustSize();

  this->updateGeometry();

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

  QPushButton *
KButtonBox::addButton(
  const KGuiItem& guiitem,
  QObject *       receiver,
  const char *    slot,
  bool            noexpand
)
{
  QPushButton * pb = addButton(guiitem, noexpand);

  if ((0 != receiver) && (0 != slot))
    QObject::connect(pb, SIGNAL(clicked()), receiver, slot);

  return pb;
}

void KButtonBox::addStretch(int scale) {
  if(scale > 0) {
    Item* const item = new Item(0);
    item->noexpand  = false;
    item->stretch = scale;
    data->buttons.append(item);
  }
}

void KButtonBox::layout() {
  // resize all buttons
  const QSize bs = bestButtonSize();

  QPtrListIterator<KButtonBox::Item> itr(data->buttons);
  Item* item;

  while ( (item = itr.current()) != 0 ) {
    QPushButton* const b = item->button;
    if(b) {
      if(item->noexpand)
	b->setFixedSize(buttonSizeHint(b));
      else
	b->setFixedSize(bs);
    }
    ++itr;
  }

  setMinimumSize(sizeHint());
}

void KButtonBox::placeButtons() {

  if(data->orientation == Horizontal) {
    // calculate free size and stretches
    int fs = width() - 2 * data->border;
    int stretch = 0;
    {
      QPtrListIterator<KButtonBox::Item> itr(data->buttons);
      Item *item;

      while ( (item = itr.current()) != 0 ) {
        QPushButton* const b = item->button;
        if(b) {
          fs -= b->width();

          // Last button?
          if(!itr.atLast())
            fs -= data->autoborder;
        } else {
          stretch +=item->stretch;
        }

        ++itr;
      }
    }

    // distribute buttons
    int x_pos = data->border;
    {
      QPtrListIterator<KButtonBox::Item> itr(data->buttons);
      Item *item;

      while ( (item = itr.current()) != 0 ) {
        QPushButton* const b = item->button;
        if(b) {
          b->move(x_pos, (height() - b->height()) / 2);

          x_pos += b->width() + data->autoborder;
        } else {
          x_pos += (int)((((double)fs) * item->stretch) / stretch);
        }

        ++itr;
      }
    }

  } else { // VERTICAL
    // calcualte free size and stretches
    int fs = height() - 2 * data->border;
    int stretch = 0;
    {
      QPtrListIterator<KButtonBox::Item> itr(data->buttons);
      Item *item;

      while ( (item = itr.current()) != 0 ) {
        QPushButton* const b = item->button;
        if(b)
          fs -= b->height() + data->autoborder;
        else
          stretch +=item->stretch;

	++itr;
      }

    }

    // distribute buttons
    int y_pos = data->border;
    {
      QPtrListIterator<KButtonBox::Item> itr(data->buttons);
      Item *item;

      while ( (item = itr.current()) != 0 ) {
        QPushButton* const b = item->button;
        if(b) {
          b->move((width() - b->width()) / 2, y_pos);

          y_pos += b->height() + data->autoborder;
        } else {
          y_pos += (int)((((double)fs) * item->stretch) / stretch);
        }

        ++itr;
      }
    }
  }
}

void KButtonBox::resizeEvent(QResizeEvent *) {
  placeButtons();
}

QSize KButtonBox::bestButtonSize() const {
  QSize s(0, 0);

  // calculate optimal size
  QPtrListIterator<KButtonBox::Item> itr(data->buttons);
  Item *item;

  while ( (item = itr.current()) != 0 ) {
    QPushButton* const b = item->button;

    if(b && !item->noexpand) {
      const QSize bs = buttonSizeHint(b);

      const int bsWidth = bs.width();
      const int bsHeight = bs.height();

      if(bsWidth > s.width())
	s.setWidth(bsWidth);
      if(bsHeight > s.height())
	s.setHeight(bsHeight);
    }
    ++itr;
  }

  return s;
}

QSize KButtonBox::sizeHint() const {
  unsigned int dw;

  if(data->buttons.isEmpty())
    return QSize(0, 0);
  else {
    dw = 2 * data->border;

    const QSize bs = bestButtonSize();

    QPtrListIterator<KButtonBox::Item> itr(data->buttons);
    Item *item;

    while ( (item = itr.current()) != 0 ) {
      QPushButton* const b = item->button;

      if(b) {
	QSize s;
	if(item->noexpand)
	  s = buttonSizeHint(b);
	else
	  s = bs;

	if(data->orientation == Horizontal)
	  dw += s.width();
	else
	  dw += s.height();

	if( !itr.atLast() )
	  dw += data->autoborder;
      }

      ++itr;
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
  const QSize ms = b->minimumSize();
  if(s.width() < minButtonWidth)
    s.setWidth(minButtonWidth);

  // allows the programmer to override the settings
  const int msWidth = ms.width();
  const int msHeight = ms.height();

  if(msWidth > s.width())
    s.setWidth(msWidth);
  if(msHeight > s.height())
    s.setHeight(msHeight);

  return s;
}

void KButtonBox::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

