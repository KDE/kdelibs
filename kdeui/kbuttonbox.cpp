/* 
 * KButtonBox class
 *
 * A container widget for buttons. Uses Qt layout control to place the
 * buttons, can handle both vertical and horizontal button placement.
 *
 * (c) 1997 Mario Weilguni <mweilguni@sime.com>
 *
 * This file is published under the GNU General Library Public License.
 * See COPYING.LIB for details.
 *
 * $Id: $
 */

#include "kbuttonbox.h"

KButtonBox::KButtonBox(QWidget *parent, int _orientation, int border) 
  :  QWidget(parent) 
{
  orientation = _orientation;
  
  // create appropriate layout
  if(_orientation == HORIZONTAL)
    tl_layout = new QHBoxLayout(this, border);
  else
    tl_layout = new QVBoxLayout(this, border);
  
  _border = border;
  buttons.setAutoDelete(TRUE);
}

KButtonBox::~KButtonBox() {
  // an empty destructor is needed for g++ if a class uses
  // a qlist, qarray or similar, otherwise it will not
  // compile
}

QPushButton *KButtonBox::addButton(const char *text, bool noexpand) {
  KButtonBoxItem *item = new KButtonBoxItem;

  item->button = new QPushButton(text, this);
  item->noexpand  = noexpand;
  buttons.append(item);
  item->button->adjustSize();
 
  // resize all buttons
  QSize bs = bestButtonSize();
  for(unsigned i = 0; i < buttons.count(); i++) {
    KButtonBoxItem *item = buttons.at(i);
    QPushButton *b = item->button;
    if(b != 0)
      if(!item->noexpand)
	b->setFixedSize(bs);
      else
	b->setFixedSize(b->sizeHint());
  }

  tl_layout->addWidget(item->button);
  
  return item->button;
}

void KButtonBox::addStretch(int scale) {
  KButtonBoxItem *item = new KButtonBoxItem;

  item->button = 0;
  item->noexpand  = FALSE;
  tl_layout->addStretch(scale);
  buttons.append(item);
}

void KButtonBox::layout() {
  setMinimumSize(sizeHint());
  tl_layout->activate();
}

QSize KButtonBox::bestButtonSize() {
  QSize s(0, 0);
  unsigned i;

  // calculate optimal size
  for(i = 0; i < buttons.count(); i++) {
    KButtonBoxItem *item = buttons.at(i);
    if(item->button != 0 && !item->noexpand) {
      if(item->button->size().width() > s.width())
	s.setWidth(item->button->size().width());
      if(item->button->size().height() > s.height())
	s.setHeight(item->button->size().height());
    }      
  }

  return s;
}

QSize KButtonBox::sizeHint() {
  unsigned i, dw;

  if(buttons.count() == 0)
    return QSize(0, 0);
  else {
    dw = _border;
    QSize bs = bestButtonSize();
    for(i = 0; i < buttons.count(); i++) {
      KButtonBoxItem *item = buttons.at(i);
      QPushButton *b = item->button;
      if(b != 0) {
	QSize s;
	if(item->noexpand)
	  s = b->sizeHint();
	else
	  s = bs;
	
	if(orientation == HORIZONTAL)
	  dw += s.width() + _border;
	else
	  dw += s.height() + _border;
      }
    }

    if(orientation == HORIZONTAL)
      return QSize(dw, bs.height() + 2 * _border);
    else
      return QSize(bs.width() + 2 * _border, dw);
  }  
}

#include "kbuttonbox.moc"
