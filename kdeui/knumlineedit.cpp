/**********************************************************************
**
** $Id: $
**
** Copyright (C) 1999 Glen Parker <glenebob@nwlink.com>
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the Free
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
*****************************************************************************/

#include <qvalidator.h>

#include "knumvalidator.h"

#include "knumlineedit.h"
#include "knumlineedit.moc"

///////////////////////////////////////////////////////////////
//  Implementation of KIntLineEdit
//

KIntLineEdit::KIntLineEdit ( QWidget * parent, const char * name )
  : QLineEdit(parent, name)
{
  __value = 0;
  setValidator(new KIntValidator(this));
  connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(__textChanged(const QString&)));
  setValue(__value);
}

KIntLineEdit::KIntLineEdit (  int value, QWidget * parent,
                              int bottom, int top, int base, const char * name )
  : QLineEdit(parent, name)
{
  __value = value;
  setValidator(new KIntValidator(bottom, top, this, base));
  connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(__textChanged(const QString&)));
  setValue(value);
}

KIntLineEdit::~KIntLineEdit ()
{}

void KIntLineEdit::setRange(int bottom, int top)
{
  ((KIntValidator*)validator())->setRange(bottom, top);
}

int KIntLineEdit::bottom() const
{
  return ((KIntValidator*)validator())->bottom();
}

int KIntLineEdit::top() const
{
  return ((KIntValidator*)validator())->top();
}

void KIntLineEdit::setBase(int base)
{
  ((KIntValidator*)validator())->setBase(base);
}

int KIntLineEdit::base() const
{
  return ((KIntValidator*)validator())->base();
}

void KIntLineEdit::setValue(int val)
{
  QString str;

  if (bottom() || top()) {
    if (val < bottom()) val = bottom();
    if (val > top()) val = top();
  }

  str.setNum(val, base());

  setText(str);
}

int KIntLineEdit::value() const
{
  return __value;
}

int KIntLineEdit::getValue() const
{
  debug("KIntLineEdit::getValue() is deprecated - use value() instead!");
  return __value;
}

void KIntLineEdit::__textChanged(const QString &newStr)
{
  QString str = newStr;
  int     val;

  validator()->fixup(str);

  val = str.toInt(0, base());

  if (bottom() || top()) {
    if (val < bottom()) val = bottom();
    if (val > top()) val = top();
  }

  if (__value != val) {
    __value = val;
    emit valueChanged(__value);
  }
}

void KIntLineEdit::keyPressEvent ( QKeyEvent *e )
{
  switch(e->key()) {
  case Key_Up:
    setValue(__value + 1);
    break;

  case Key_Down:
    setValue(__value - 1);
    break;

  case Key_Prior:
    setValue(__value + base());
    break;

  case Key_Next:
    setValue(__value - base());
    break;

  default:
    QLineEdit::keyPressEvent(e);
    break;

  }
}

void KIntLineEdit::focusOutEvent ( QFocusEvent *e )
{
  setValue(__value);
  QLineEdit::focusOutEvent(e);
}

///////////////////////////////////////////////////////////////
//  Implementation of KFloatLineEdit
//

KFloatLineEdit::KFloatLineEdit ( QWidget * parent, const char * name )
  : QLineEdit(parent, name)
{
  __value = 0;
  setValidator(new KFloatValidator(this));
  connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(__textChanged(const QString&)));
  setValue(__value);
}

KFloatLineEdit::KFloatLineEdit (  double value, QWidget * parent,
                                  double bottom, double top, const char * name )
  : QLineEdit(parent, name)
{
  __value = 0;
  setValidator(new KFloatValidator(bottom, top, this));
  connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(__textChanged(const QString&)));
  setValue(value);
}

KFloatLineEdit::~KFloatLineEdit ()
{}

void KFloatLineEdit::setRange(float bottom, float top)
{
  ((KFloatValidator*)validator())->setRange(bottom, top);
}

float KFloatLineEdit::bottom() const
{
  return ((KFloatValidator*)validator())->bottom();
}

float KFloatLineEdit::top() const
{
  return ((KFloatValidator*)validator())->top();
}

void KFloatLineEdit::setValue(double val)
{
  QString str;

  if (bottom() || top()) {
    if (val < bottom()) val = bottom();
    if (val > top()) val = top();
  }

  str.setNum(val);

  setText(str);
}

double KFloatLineEdit::value() const
{
  return __value;
}

void KFloatLineEdit::__textChanged(const QString &newStr)
{
  QString     str = newStr;
  double      val;

  validator()->fixup(str);

  val = str.toDouble();

  if (bottom() || top()) {
    if (val < bottom()) val = bottom();
    if (val > top()) val = top();
  }

  if (__value != val) {
    __value = val;
    emit valueChanged(__value);
  }
}

void KFloatLineEdit::keyPressEvent ( QKeyEvent *e )
{
  switch(e->key()) {
  case Key_Up:
    setValue(__value + 1);
    break;

  case Key_Down:
    setValue(__value - 1);
    break;

  case Key_Prior:
    setValue(__value + 10);
    break;

  case Key_Next:
    setValue(__value - 10);
    break;

  default:
    QLineEdit::keyPressEvent(e);
    break;

  }
}

void KFloatLineEdit::focusOutEvent ( QFocusEvent *e )
{
  setValue(__value);
  QLineEdit::focusOutEvent(e);
}


