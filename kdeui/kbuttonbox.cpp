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
 */

#include "kbuttonbox.h"
#include <stdio.h>

// taken from Qt source
const int extraMotifWidth = 10;
const int extraMotifHeight = 10;

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
  
  // create appropriate layout
//   if(_orientation == HORIZONTAL)
//     tl_layout = new QHBoxLayout(this, _border, _autoborder);
//   else
//     tl_layout = new QVBoxLayout(this, _border, _autoborder);
  
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
	b->setFixedSize(b->sizeHint());
    }
  }  

  setMinimumSize(sizeHint());
  placeButtons();
//   tl_layout->activate();
}

void KButtonBox::placeButtons() {
  unsigned i;

  if(orientation == HORIZONTAL) {
    // calcualte free size and stretches
    int fs = width() - 2 * _border;
    int stretch = 0;
    for(i = 0; i < buttons.count(); i++) {
      KButtonBoxItem *item = buttons.at(i);
      if(item->button != 0) 
	fs -= item->button->width() + _autoborder;
      else
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

QSize KButtonBox::bestButtonSize() {
  QSize s(0, 0);
  unsigned i;

  // calculate optimal size
  for(i = 0; i < buttons.count(); i++) {
    KButtonBoxItem *item = buttons.at(i);
    QPushButton *b = item->button;
 
    if(b != 0 && !item->noexpand) {      
      QSize bs = b->sizeHint();
      fflush(stdout);
      if(b->style() == MotifStyle && b->isDefault()) {
	// this is a motif default button, remove the
	// space for the default ring
	bs.setWidth(bs.width() - extraMotifWidth);
	bs.setHeight(bs.height() - extraMotifHeight);
      }

      if(bs.width() > s.width())
	s.setWidth(bs.width());
      if(bs.height() > s.height())
	s.setHeight(bs.height());      
    }
  }

  return s;
}

QSize KButtonBox::sizeHint() {
  unsigned i, dw;

  if(buttons.count() == 0)
    return QSize(0, 0);
  else {
    dw = 2*_border;

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
	  dw += s.width() + _autoborder;
	else
	  dw += s.height() + _autoborder;
      }
    }

    if(orientation == HORIZONTAL) {
      if(style() == MotifStyle) 	
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

#include "kbuttonbox.moc"
