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

#ifndef __KNUMLINEEDIT_H
#define __KNUMLINEEDIT_H

#include <klined.h>

/**
  LineEdit for integer data.  Uses KIntValidator validator class.
  This class supports an optional enforced range, and can display the integer
  in any base between 2 and 36.
  The displayed value is validated in real time if possible, but there are
  some situations where this cannot work.  Therefore, the display will
  always be validated and fixed up when the widget loses focus.

  @author Glen Parker <glenebob@nwlink.com>
  @version 0.0.2
  @include knumlineedit.h
*/
class KIntLineEdit : public KLineEdit {

  Q_OBJECT

  public:
    /**
      Constructor.  The parameters are passed directly to the super class.
    */
    KIntLineEdit ( QWidget * parent, const char * name = 0 );
    /**
      Constructor.  The parent and name parameters are passed directly to the super class.
      @param value   Set the initial numeric value.
      @param bottom  Set the minimum value allowed.
      @param top     Set the maximum value allowed.
      @param base    Set the numeric base value.
    */
    KIntLineEdit ( int value, QWidget * parent, int bottom = 0, int top = 0, int base = 10, const char * name = 0 );
    /**
      Destructor.
    */
    virtual ~KIntLineEdit ();

    /**
      Set the minimum and maximum values allowed.
    */
    virtual void setRange(int bottom = 0, int top = 0);
    /**
      Return the current minimum value allowed.
    */
    virtual int bottom() const;
    /**
      Return the current maximum value allowed.
    */
    virtual int top() const;

    /**
      Set the numeric base value.  This may be any number between 2 and 36.
    */
    virtual void setBase(int base = 10);
    /**
      Return the current numeric base value.
    */
    virtual int base() const;

    /**
      Return the current validated numeric value.
      The value returned will represent the displayed value, unless the displayed value
      falls outside of the allowed range.  In other words, this function always returns
      a validated integer, regardless of what is displayed.
    */
    virtual int value() const;
    /**
      Same as value, for compatibility with old KIntLineEdit.
      Will emit warnings if used.
      @deprecated
    */
    virtual int getValue() const;

  public slots:
    /**
      Set the numeric value.  The new value is validated before being displayed.
    */
    virtual void setValue(int);

  signals:
    /**
      Emitted whenever the validated value has changed.
    */
    void valueChanged(int);

  protected:
    void keyPressEvent ( QKeyEvent * );
    void focusOutEvent ( QFocusEvent * );

    int     __value;

  protected slots:
    void __textChanged(const QString&);

};

/**
  LineEdit for double numeric data.  Uses KFloatValidator validator class.
  This class supports an optional enforced range.
  The data type used by this class is double.
  The displayed value is validated in real time if possible, but there are
  some situations where this cannot work.  Therefore, the display will
  always be validated and fixed up when the widget loses focus.

  @author Glen Parker <glenebob@nwlink.com>
  @version 0.0.3
  @include knumlineedit.h
*/
class KFloatLineEdit : public KLineEdit {

  Q_OBJECT

  public:
    /**
      Constructor.  The parameters are passed directly to the super class.
    */
    KFloatLineEdit ( QWidget * parent, const char * name=0 );
    /**
      Constructor.  The parent and name parameters are passed directly to the super class.
      @param value   Set the initial numeric value.
      @param bottom  Set the minimum value allowed.
      @param top     Set the maximum value allowed.
    */
    KFloatLineEdit ( double value, QWidget * parent, double bottom = 0, double top = 0, const char * name=0 );
    /**
      Destructor.
    */
    virtual ~KFloatLineEdit ();

    /**
      Set the minimum and maximum values allowed.
    */
    virtual void setRange(double bottom = 0, double top = 0);
    /**
      Return the current minimum value allowed.
    */
    virtual double bottom() const;
    /**
      Return the current maximum value allowed.
    */
    virtual double top() const;
    /**
      Return the current validated numeric value.
      The value returned will represent the displayed value, unless the displayed value
      falls outside of the allowed range.  In other words, this function always returns
      a validated number, regardless of what is displayed.
    */
    virtual double value() const;

  public slots:
    /**
      Set the numeric value.  The new value is validated before being displayed.
    */
    virtual void setValue(double);

  signals:
    /**
      Emitted whenever the validated value has changed.
    */
    void valueChanged(double);

  protected:
    virtual void keyPressEvent ( QKeyEvent * );
    virtual void focusOutEvent ( QFocusEvent * );

    double     __value;

  protected slots:
    void __textChanged(const QString&);

};


#endif

