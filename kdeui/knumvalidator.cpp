/**********************************************************************
**
** $Id$
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

///////////////////////////////////////////////////////////////
//  Implementation of KIntValidator
//

KIntValidator::KIntValidator ( QWidget * parent, int base, const char * name )
  : QValidator(parent, name)
{
  _base = base;
  if (_base < 2) _base = 2;
  if (_base > 36) _base = 36;

  _min = _max = 0;
}

KIntValidator::KIntValidator ( int bottom, int top, QWidget * parent, int base, const char * name )
  : QValidator(parent, name)
{
  _base = base;
  if (_base > 36) _base = 36;

  _min = bottom;
  _max = top;
}

KIntValidator::~KIntValidator ()
{}

QValidator::State KIntValidator::validate ( QString &str, int & ) const
{
  bool ok;
  int  val = 0;
  QString newStr;

  newStr = str.stripWhiteSpace();
  if (_base > 10)
    newStr = newStr.upper();

  if (newStr == "-") // a special case
    if ((_min || _max) && _min >= 0)
      ok = false;
    else
      return QValidator::Acceptable;
  else if (newStr.length())
    val = newStr.toInt(&ok, _base);
  else {
    val = 0;
    ok = true;
  }

  if (! ok)
    return QValidator::Invalid;

  if ((! _min && ! _max) || (val >= _min && val <= _max))
    return QValidator::Acceptable;

  if (_max && _min >= 0 && val < 0)
    return QValidator::Invalid;

  return QValidator::Valid;
}

void KIntValidator::fixup ( QString &str ) const
{
  int                dummy;
  int                val;
  QValidator::State  state;

  state = validate(str, dummy);

  if (state == QValidator::Invalid || state == QValidator::Acceptable)
    return;

  if (! _min && ! _max)
    return;

  val = str.toInt(0, _base);

  if (val < _min) val = _min;
  if (val > _max) val = _max;

  str.setNum(val, _base);
}

void KIntValidator::setRange ( int bottom, int top )
{
  _min = bottom;
  _max = top;

	if (_max < _min)
		_max = _min;
}

void KIntValidator::setBase ( int base )
{
  _base = base;
  if (_base < 2) _base = 2;
}

int KIntValidator::bottom () const
{
  return _min;
}

int KIntValidator::top () const
{
  return _max;
}

int KIntValidator::base () const
{
  return _base;
}


///////////////////////////////////////////////////////////////
//  Implementation of KIntValidator
//

KFloatValidator::KFloatValidator ( QWidget * parent, const char * name )
  : QValidator(parent, name)
{
  _min = _max = 0;
}

KFloatValidator::KFloatValidator ( double bottom, double top, QWidget * parent, const char * name )
  : QValidator(parent, name)
{
  _min = bottom;
  _max = top;
}

KFloatValidator::~KFloatValidator ()
{}

QValidator::State KFloatValidator::validate ( QString &str, int & ) const
{
  bool    ok;
  double  val = 0;

  QString newStr;

  newStr = str.stripWhiteSpace();

  if (newStr == "-") // a special case
    if ((_min || _max) && _min >= 0)
      ok = false;
    else
      return QValidator::Acceptable;
  else if (newStr == ".") // another special case
    return QValidator::Acceptable;
  else if (newStr.length())
    val = newStr.toDouble(&ok);
  else {
    val = 0;
    ok = true;
  }

  if (! ok)
    return QValidator::Invalid;

  if ((! _min && ! _max) || (val >= _min && val <= _max))
    return QValidator::Acceptable;

  if (_max && _min >= 0 && val < 0)
    return QValidator::Invalid;

  return QValidator::Valid;
}

void KFloatValidator::fixup ( QString &str ) const
{
  int                dummy;
  double             val;
  QValidator::State  state;

  state = validate(str, dummy);

  if (state == QValidator::Invalid || state == QValidator::Acceptable)
    return;

  if (! _min && ! _max)
    return;

  val = str.toDouble();

  if (val < _min) val = _min;
  if (val > _max) val = _max;

  str.setNum(val);
}

void KFloatValidator::setRange ( double bottom, double top )
{
  _min = bottom;
  _max = top;

	if (_max < _min)
		_max = _min;
}

double KFloatValidator::bottom () const
{
  return _min;
}

double KFloatValidator::top () const
{
  return _max;
}

