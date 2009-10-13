/**********************************************************************
**
** Copyright (C) 1999 Glen Parker <glenebob@nwlink.com>
** Copyright (C) 2002 Marc Mutz <mutz@kde.org>
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

#ifndef KNUMVALIDATOR_H
#define KNUMVALIDATOR_H

#include <kdeui_export.h>

#include <QtGui/QValidator>

class QWidget;
class QString;

/**
 * QValidator for integers.

  This can be used by QLineEdit or subclass to provide validated
  text entry.  Can be provided with a base value (default is 10), to allow
  the proper entry of hexadecimal, octal, or any other base numeric data.

  @author Glen Parker <glenebob@nwlink.com>
  @version 0.0.1
*/
class KDEUI_EXPORT KIntValidator : public QValidator {

  public:
    /**
     * Constructor.  Also sets the base value.
     */
    explicit KIntValidator ( QWidget * parent, int base = 10 );
    /**
     * Constructor.  Also sets the minimum, maximum, and numeric base values.
     */
    KIntValidator ( int bottom, int top, QWidget * parent, int base = 10 );
    /**
     * Destructs the validator.
     */
    virtual ~KIntValidator ();
    /**
     * Validates the text, and return the result.  Does not modify the parameters.
     */
    virtual State validate ( QString &, int & ) const;
    /**
     * Fixes the text if possible, providing a valid string.  The parameter may be modified.
     */
    virtual void fixup ( QString & ) const;
    /**
     * Sets the minimum and maximum values allowed. 
     * If @p top is greater than @p bottom, it is set to the value of @p bottom.
     */
    virtual void setRange ( int bottom, int top );
    /**
     * Sets the numeric base value. @p base must be between 2 and 36.
     */
    virtual void setBase ( int base );
    /**
     * Returns the current minimum value allowed.
     */
    virtual int bottom () const;
    /**
     * Returns the current maximum value allowed.
     */
    virtual int top () const;
    /**
     * Returns the current numeric base.
     */
    virtual int base () const;

  private:
    class KIntValidatorPrivate;
    KIntValidatorPrivate * const d;
};

/**
 \brief QValidator for floating point entry (Obsolete)

  @obsolete Use KDoubleValidator

  Extends the QValidator class to properly validate double numeric data.
  This can be used by QLineEdit or subclass to provide validated
  text entry.

  @author Glen Parker <glenebob@nwlink.com>
  @version 0.0.1
*/
class KDEUI_EXPORT KFloatValidator : public QValidator {

  public:
    /**
     * Constructor.
     */
    explicit KFloatValidator ( QWidget * parent );
    /**
     * Constructor.  Also sets the minimum and maximum values.
     */
    KFloatValidator ( double bottom, double top, QWidget * parent );
    /**
     * Constructor.  Sets the validator to be locale aware if @p localeAware is true.
     */
    KFloatValidator ( double bottom, double top, bool localeAware, QWidget * parent );
    /**
     * Destructs the validator.
     */
    virtual ~KFloatValidator ();
    /**
     * Validates the text, and return the result. Does not modify the parameters.
     */
    virtual State validate ( QString &, int & ) const;
    /**
     * Fixes the text if possible, providing a valid string. The parameter may be modified.
     */
    virtual void fixup ( QString & ) const;
    /**
     * Sets the minimum and maximum value allowed.
     */
    virtual void setRange ( double bottom, double top );
    /**
     * Returns the current minimum value allowed.
     */
    virtual double bottom () const;
    /**
     * Returns the current maximum value allowed.
     */
    virtual double top () const;
    /**
     * Sets the validator to be locale aware if @p is true. In this case, the
     * character KLocale::decimalSymbol() from the global locale is recognized
     * as decimal separator.
     */
    void setAcceptLocalizedNumbers(bool b);
    /**
     * Returns true if the validator is locale aware.
     * @see setAcceptLocalizedNumbers().
     */
    bool acceptLocalizedNumbers() const;

 private:
    class KFloatValidatorPrivate;
    KFloatValidatorPrivate * const d;
};

/**
   @short A locale-aware QDoubleValidator

   KDoubleValidator extends QDoubleValidator to be
   locale-aware. That means that - subject to not being disabled -
   KLocale::decimalSymbol(), KLocale::thousandsSeparator()
   and KLocale::positiveSign() and KLocale::negativeSign()
   are respected.

   @author Marc Mutz <mutz@kde.org>
   @see KIntValidator
**/

class KDEUI_EXPORT KDoubleValidator : public QDoubleValidator {
  Q_OBJECT
  Q_PROPERTY( bool acceptLocalizedNumbers READ acceptLocalizedNumbers WRITE setAcceptLocalizedNumbers )
public:
  /** Constuct a locale-aware KDoubleValidator with default range
      (whatever QDoubleValidator uses for that) and parent @p
      parent */
  explicit KDoubleValidator( QObject * parent );
  /** Constuct a locale-aware KDoubleValidator for range [@p bottom,@p
      top] and a precision of @p decimals decimals after the decimal
      point.  */
  KDoubleValidator( double bottom, double top, int decimals,
		    QObject * parent );
  /** Destructs the validator.
   */
  virtual ~KDoubleValidator();

  /** Overloaded for internal reasons. The API is not affected. */
  virtual QValidator::State validate( QString & input, int & pos ) const;

  /** @return whether localized numbers are accepted (default: true) */
  bool acceptLocalizedNumbers() const;
  /** Sets whether to accept localized numbers (default: true) */
  void setAcceptLocalizedNumbers( bool accept );

private:
  typedef QDoubleValidator base;
  class KDoubleValidatorPrivate;
  KDoubleValidatorPrivate * const d;
};

#endif
