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

#ifndef __KNUMLINEEDIT_H
#define __KNUMLINEEDIT_H

#include <qlineedit.h>

/**
  LineEdit for integer data.  Uses KIntValidator validator class.
  This class can use any numeric value between 2 and 36.

  @author Glen Parker <glenebob@nwlink.com>
  @version 0.0.1
*/
class KIntLineEdit : public QLineEdit {

  Q_OBJECT

  public:
    /**
      Constructor.  The parameters are passed directly to the super class.
    */
    KIntLineEdit ( QWidget * parent, const char * name=0 );
    /**
      Constructor.  The parent and name parameters are passed directly to the super class.
      @param value   Set the initial numeric value.
      @param bottom  Set the minimum value allowed.
      @param top     Set the maximum value allowed.
      @param base    Set the numeric base value.
    */
    KIntLineEdit ( int value, QWidget * parent, int bottom = 0, int top = 0, int base= 10, const char * name=0 );
    /**
      Destructor.
    */
    virtual ~KIntLineEdit ();

    /**
      Set the minimum and maximum integer value allowed.
    */
    virtual void setRange(int bottom = 0, int top = 0);
    /**
      Returns the current minimum value allowed.
    */
    virtual int bottom() const;
    /**
      Returns the current maximum value allowed.
    */
    virtual int top() const;

    /**
      Set the numeric base value.
    */
    virtual void setBase(int base = 10);
    /**
      Returns the current numeric base value.
    */
    virtual int base() const;

    /**
      Returns the current validated numeric value.
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
  LineEdit for double data.  Uses KFloatValidator validator class.

  @author Glen Parker <glenebob@nwlink.com>
  @version 0.0.1
*/
class KFloatLineEdit : public QLineEdit {

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
      Set the minimum and maximum integer value allowed.
    */
    virtual void setRange(float bottom = 0, float top = 0);
    /**
      Returns the current minimum value allowed.
    */
    virtual float bottom() const;
    /**
      Returns the current maximum value allowed.
    */
    virtual float top() const;
    /**
      Returns the current validated numeric value.
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

