/**********************************************************************
**
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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
*****************************************************************************/

#include "knumvalidator.h"

#include <QtGui/QWidget>
#include <QtCore/QCharRef>

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

///////////////////////////////////////////////////////////////
//  Implementation of KIntValidator
//
class KIntValidator::KIntValidatorPrivate
{
public:
    KIntValidatorPrivate()
     : _base(0), _min(0), _max(0)
    {}
    int _base;
    int _min;
    int _max;
};

KIntValidator::KIntValidator ( QWidget * parent, int base )
  : QValidator(parent), d(new KIntValidatorPrivate)
{
  setBase(base);
}

KIntValidator::KIntValidator ( int bottom, int top, QWidget * parent, int base )
  : QValidator(parent), d(new KIntValidatorPrivate)
{
  setBase(base);
  setRange(bottom, top);
}

KIntValidator::~KIntValidator ()
{
    delete d;
}

QValidator::State KIntValidator::validate ( QString &str, int & ) const
{
  bool ok;
  int  val = 0;
  QString newStr;

  newStr = str.trimmed();
  if (d->_base > 10)
    newStr = newStr.toUpper();

  if (newStr == QLatin1String("-")) {// a special case
    if ((d->_min || d->_max) && d->_min >= 0)
      ok = false;
    else
      return QValidator::Acceptable;
  }
  else if (!newStr.isEmpty())
    val = newStr.toInt(&ok, d->_base);
  else {
    val = 0;
    ok = true;
  }

  if (! ok)
    return QValidator::Invalid;

  if ((! d->_min && ! d->_max) || (val >= d->_min && val <= d->_max))
    return QValidator::Acceptable;

  if (d->_max && d->_min >= 0 && val < 0)
    return QValidator::Invalid;

  return QValidator::Intermediate;
}

void KIntValidator::fixup ( QString &str ) const
{
  int                dummy;
  int                val;
  QValidator::State  state;

  state = validate(str, dummy);

  if (state == QValidator::Invalid || state == QValidator::Acceptable)
    return;

  if (! d->_min && ! d->_max)
    return;

  val = str.toInt(0, d->_base);

  if (val < d->_min) val = d->_min;
  if (val > d->_max) val = d->_max;

  str.setNum(val, d->_base);
}

void KIntValidator::setRange ( int bottom, int top )
{
  d->_min = bottom;
  d->_max = top;

	if (d->_max < d->_min)
		d->_max = d->_min;
}

void KIntValidator::setBase ( int base )
{
  d->_base = base;
  if (d->_base < 2) d->_base = 2;
  if (d->_base > 36) d->_base = 36;
}

int KIntValidator::bottom () const
{
  return d->_min;
}

int KIntValidator::top () const
{
  return d->_max;
}

int KIntValidator::base () const
{
  return d->_base;
}


///////////////////////////////////////////////////////////////
//  Implementation of KFloatValidator
//

class KFloatValidator::KFloatValidatorPrivate
{
public:
    KFloatValidatorPrivate()
     : acceptLocalizedNumbers(false), _min(0), _max(0)
    {}
    bool acceptLocalizedNumbers;
    double _min;
    double _max;
};


KFloatValidator::KFloatValidator ( QWidget * parent )
  : QValidator(parent), d(new KFloatValidatorPrivate)
{
    d->acceptLocalizedNumbers=false;
}

KFloatValidator::KFloatValidator ( double bottom, double top, QWidget * parent )
  : QValidator(parent), d(new KFloatValidatorPrivate)
{
    d->acceptLocalizedNumbers=false;
    setRange(bottom, top);
}

KFloatValidator::KFloatValidator ( double bottom, double top, bool localeAware, QWidget * parent )
  : QValidator(parent), d(new KFloatValidatorPrivate)
{
    d->acceptLocalizedNumbers = localeAware;
    setRange(bottom, top);
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

QValidator::State KFloatValidator::validate ( QString &str, int & ) const
{
  bool    ok;
  double  val = 0;
  QString newStr;
  newStr = str.trimmed();

  if (newStr == QLatin1String("-")) {// a special case
    if ((d->_min || d->_max) && d->_min >= 0)
      ok = false;
    else
      return QValidator::Acceptable;
  }
  else if (newStr == QLatin1String(".") || (d->acceptLocalizedNumbers && newStr==KGlobal::locale()->decimalSymbol())) // another special case
    return QValidator::Acceptable;
  else if (!newStr.isEmpty())
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

  if (( !d->_min && !d->_max) || (val >= d->_min && val <= d->_max))
    return QValidator::Acceptable;

  if (d->_max && d->_min >= 0 && val < 0)
    return QValidator::Invalid;

  if ( (d->_min || d->_max) && (val < d->_min || val > d->_max))
    return QValidator::Invalid;

  return QValidator::Intermediate;
}

void KFloatValidator::fixup ( QString &str ) const
{
  int                dummy;
  double             val;
  QValidator::State  state;

  state = validate(str, dummy);

  if (state == QValidator::Invalid || state == QValidator::Acceptable)
    return;

  if (! d->_min && ! d->_max)
    return;

  val = str.toDouble();

  if (val < d->_min) val = d->_min;
  if (val > d->_max) val = d->_max;

  str.setNum(val);
}

void KFloatValidator::setRange ( double bottom, double top )
{
  d->_min = bottom;
  d->_max = top;

	if (d->_max < d->_min)
		d->_max = d->_min;
}

double KFloatValidator::bottom () const
{
  return d->_min;
}

double KFloatValidator::top () const
{
  return d->_max;
}




///////////////////////////////////////////////////////////////
//  Implementation of KDoubleValidator
//

class KDoubleValidator::KDoubleValidatorPrivate {
public:
  KDoubleValidatorPrivate( bool accept=true ) : acceptLocalizedNumbers( accept ) {}

  bool acceptLocalizedNumbers;
};

KDoubleValidator::KDoubleValidator( QObject * parent )
  : QDoubleValidator( parent ), d(  new KDoubleValidatorPrivate() )
{
}

KDoubleValidator::KDoubleValidator( double bottom, double top, int decimals,
  QObject * parent )
  : QDoubleValidator( bottom, top, decimals, parent ), d( new KDoubleValidatorPrivate() )
{
}

KDoubleValidator::~KDoubleValidator()
{
  delete d;
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
      for ( int idx = s.indexOf( p ) ; idx >= 0 ; idx = s.indexOf( p, idx ) )
        s.remove( idx, p.length() );


    if ( !t.isEmpty() )
      for ( int idx = s.indexOf( t ) ; idx >= 0 ; idx = s.indexOf( t, idx ) )
        s.remove( idx, t.length() );

    // then, replace the d's and n's
    if ( ( !n.isEmpty() && n.indexOf('.') != -1 ) ||
       ( !d.isEmpty() && d.indexOf('-') != -1 ) ) {
      // make sure we don't replace something twice:
      kWarning() << "KDoubleValidator: decimal symbol contains '-' or "
                    "negative sign contains '.' -> improve algorithm" << endl;
      return Invalid;
    }

    if ( !d.isEmpty() && d != "." )
      for ( int idx = s.indexOf( d ) ; idx >= 0 ; idx = s.indexOf( d, idx + 1 ) )
        s.replace( idx, d.length(), '.');

    if ( !n.isEmpty() && n != "-" )
      for ( int idx = s.indexOf( n ) ; idx >= 0 ; idx = s.indexOf( n, idx + 1 ) )
        s.replace( idx, n.length(), '-' );
  }

  return base::validate( s, p );
}

#include "knumvalidator.moc"
