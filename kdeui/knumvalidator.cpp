/**********************************************************************
**
** $Id$
**
** KIntValidator, KFloatValidator:
**   Copyright (C) 1999 Glen Parker <glenebob@nwlink.com>
** KDoubleValidator:
**   Copyright (c) 2002 Marc Mutz <mutz@kde.org>
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

#include <qwidget.h>
#include <qstring.h>

#include "knumvalidator.h"
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

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

  if (newStr == QString::fromLatin1("-")) // a special case
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
//  Implementation of KFloatValidator
//

class KFloatValidatorPrivate
{
public:
    KFloatValidatorPrivate()
    {
    }
    ~KFloatValidatorPrivate()
    {
    }
    bool acceptLocalizedNumbers;
};


KFloatValidator::KFloatValidator ( QWidget * parent, const char * name )
  : QValidator(parent, name)
{
    d = new KFloatValidatorPrivate;
    d->acceptLocalizedNumbers=false;
    _min = _max = 0;
}

KFloatValidator::KFloatValidator ( double bottom, double top, QWidget * parent, const char * name )
  : QValidator(parent, name)
{
    d = new KFloatValidatorPrivate;
    d->acceptLocalizedNumbers=false;
    _min = bottom;
    _max = top;
}

KFloatValidator::KFloatValidator ( double bottom, double top, bool localeAware, QWidget * parent, const char * name )
  : QValidator(parent, name)
{
    d = new KFloatValidatorPrivate;
    d->acceptLocalizedNumbers = localeAware;
    _min = bottom;
    _max = top;
}

KFloatValidator::~KFloatValidator ()
{
     delete d;
}

void KFloatValidator::setAcceptLocalizedNumbers(bool _b)
{
    d->acceptLocalizedNumbers=_b;
}

bool KFloatValidator::acceptLocalizedNumbers() const
{
    return d->acceptLocalizedNumbers;
}

#include <kdebug.h>
QValidator::State KFloatValidator::validate ( QString &str, int & ) const
{
  bool    ok;
  double  val = 0;
  QString newStr;
  newStr = str.stripWhiteSpace();

  if (newStr == QString::fromLatin1("-")) // a special case
    if ((_min || _max) && _min >= 0)
      ok = false;
    else
      return QValidator::Acceptable;
  else if (newStr == QString::fromLatin1(".") || (d->acceptLocalizedNumbers && newStr==KGlobal::locale()->decimalSymbol())) // another special case
    return QValidator::Acceptable;
  else if (newStr.length())
  {
    val = newStr.toDouble(&ok);
    if(!ok && d->acceptLocalizedNumbers)
       val= KGlobal::locale()->readNumber(newStr,&ok);
  }
  else {
    val = 0;
    ok = true;
  }

  if (! ok)
    return QValidator::Invalid;

  if (( !_min && !_max) || (val >= _min && val <= _max))
    return QValidator::Acceptable;

  if (_max && _min >= 0 && val < 0)
    return QValidator::Invalid;

  if ( (_min || _max) && (val < _min || val > _max))
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




///////////////////////////////////////////////////////////////
//  Implementation of KDoubleValidator
//

struct KDoubleValidator::Private {
  Private( bool accept=true ) : acceptLocalizedNumbers( accept ) {}

  bool acceptLocalizedNumbers;
};

KDoubleValidator::KDoubleValidator( QObject * parent, const char * name )
  : QDoubleValidator( parent, name ), d( 0 )
{
  d = new Private();
}

KDoubleValidator::KDoubleValidator( double bottom, double top, int decimals,
				    QObject * parent, const char * name )
  : QDoubleValidator( bottom, top, decimals, parent, name ), d( 0 )
{
  d = new Private();
}

bool KDoubleValidator::acceptLocalizedNumbers() const {
  return d->acceptLocalizedNumbers;
}

void KDoubleValidator::setAcceptLocalizedNumbers( bool accept ) {
  d->acceptLocalizedNumbers = accept;
}

QValidator::State KDoubleValidator::validate( QString & input, int & p ) const {
  QString s = input;
  if ( acceptLocalizedNumbers() ) {
    KLocale * l = KGlobal::locale();
    // ok, we have to re-format the number to have:
    // 1. decimalSymbol == '.'
    // 2. negativeSign  == '-'
    // 3. positiveSign  == <empty>
    // 4. thousandsSeparator() == <empty> (we don't check that there
    //    are exactly three decimals between each separator):
    QString d = l->decimalSymbol(),
            n = l->negativeSign(),
            p = l->positiveSign(),
            t = l->thousandsSeparator();
    // first, delete p's and t's:
    if ( !p.isEmpty() )
      for ( int idx = s.find( p ) ; idx >= 0 ; idx = s.find( p, idx ) )
	s.remove( idx, p.length() );
	

    if ( !t.isEmpty() )
      for ( int idx = s.find( t ) ; idx >= 0 ; idx = s.find( t, idx ) )
	s.remove( idx, t.length() );

    // then, replace the d's and n's
    if ( ( !n.isEmpty() && n.find('.') != -1 ) ||
	 ( !d.isEmpty() && d.find('-') != -1 ) ) {
      // make sure we don't replace something twice:
      kdWarning() << "KDoubleValidator: decimal symbol contains '-' or "
		     "negative sign contains '.' -> improve algorithm" << endl;
      return Invalid;
    }

    if ( !d.isEmpty() && d != "." )
      for ( int idx = s.find( d ) ; idx >= 0 ; idx = s.find( d, idx + 1 ) )
	s.replace( idx, d.length(), ".");

    if ( !n.isEmpty() && n != "-" )
      for ( int idx = s.find( n ) ; idx >= 0 ; idx = s.find( n, idx + 1 ) )
	s.replace( idx, n.length(), "-" );
  }

  return base::validate( s, p );
}

#include "knumvalidator.moc"
