/****************************************************************************
** $Id$
**
** Implementation of QGuardedPtr class
**
** Created : 990929
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qguardedptr.h"

// REVISED: warwick
    
/*!
  \class QGuardedPtr qguardedptr.h
  \brief The QGuardedPtr class is a template class that provides guarded pointers to QObjects

  \ingroup kernel

  A guarded pointer, QGuardedPtr\<<em>X</em>\>,
  behaves like a normal C++ pointer \e X*, except that
  it is automatically set to null when the referenced object is destroyed,
  unlike normal C++ pointers which become "dangling pointers" in that case.
  \e X must be a subclass of QObject.

  Guarded pointers are useful whenever you need to store a pointer to a
  QObject that is owned by someone else and therefore might be
  destroyed while you still keep a reference to it. You can safely test
  the pointer for validity.

  Example:
  \code
      QGuardedPtr<QFrame> label = new QLabel( 0,"label" );
      label->setText("I like guarded pointers");

      delete (QLabel*) label; // emulate somebody destroying the label

      if ( label)
	  label->show();
      else 
	  qDebug("The label has been destroyed");
  \endcode

  The program will output
  \code
      The label has been destroyed
  \endcode
  rather than dereferencing an invalid address in \c label->show().

  The functions and operators available with a QGuardedPtr are the same
  as those available with a normal unguarded pointer, except the pointer
  arithmetic operators, ++, --, -, and +, which are normally only used with
  arrays of objects. Use them like normal pointers and you will not need
  to read this class documentation.
  
  For creating guarded pointers, you can construct or assign to them
  from an X* or from another guarded pointer of the same type. You can
  compare them with each other for equality (==) and inequality (!=),
  or test for null with isNull() and for non-null with the bool() casting
  operator.  Finally, you can dereference
  them using either the \c *x or the \c x->member notation.

  A guarded pointer will automatically cast to an X* so you can freely
  mix guarded and unguarded pointers. This means that if you have a
  QGuardedPtr<QWidget>, then you can pass it to a function that
  requires a QWidget*.  For this reason, it is of little value to declare
  functions to take a QGuardedPtr as a parameter - just use normal pointers.
  Use a QGuardedPtr when you are storing a pointer over time.

  Note again that class \e X must inherit QObject or a compilation or link
  error will result.
*/

/*!
  \fn QGuardedPtr::QGuardedPtr()

  Constructs a null guarded pointer.

  \sa isNull()
*/

/*!
  \fn QGuardedPtr::QGuardedPtr( T* p )

  Constructs a guarded pointer that points to same object as is
  pointed to by \a p.
*/

/*!
  \fn QGuardedPtr::QGuardedPtr(const QGuardedPtr<T> &p)

  Copy one guarded pointer from another. The constructed guarded pointer
  points to the same object that \a p pointed to (possibly null).
*/

/*!
  \fn QGuardedPtr::~QGuardedPtr()

  Destructs the guarded pointer.
  Note that the object \e pointed \e to by the pointer
  is \e not destructed, just as is the case with a normal unguarded pointer.
*/

/*!
  \fn QGuardedPtr<T>& QGuardedPtr::operator=(const QGuardedPtr<T> &p)

  Assignment operator. This guarded pointer then points to the same
  object as does \a p.
*/

/*!
  \fn QGuardedPtr<T> & QGuardedPtr::operator=(T* p)
 
  Assignment operator.  This guarded pointer then points to same object as is
  pointed to by \a p.
*/

/*!
  \fn bool QGuardedPtr::operator==( const QGuardedPtr<T> &p ) const 

  Equality operator, implements traditional pointer semantics: returns
  TRUE if both \a p and this are null, or if both \a p and this point
  to the same object.

  \sa operator!=
*/

/*!
  \fn bool QGuardedPtr::operator!= ( const QGuardedPtr<T>& p ) const 

  Unequality operator, implements pointer semantics, the negation
  of operator==.
*/

/*!
  \fn bool QGuardedPtr::isNull() const 

  Returns \c TRUE if the referenced object has been destroyed or if there is
  no referenced object.
*/

/*!
  \fn T* QGuardedPtr::operator->()

  Overloaded arrow operator, implements pointer semantics. Just use this
  operator as you would with a normal C++ pointer.
*/

/*!
  \fn const T* QGuardedPtr::operator->() const

  Overloaded arrow operator, implements pointer semantics. Just use this
  operator as you would with a normal C++ pointer.
*/

/*!
  \fn T& QGuardedPtr::operator*() 

  Dereference operator, implements pointer semantics. Just use this
  operator as you would with a normal C++ pointer.
*/

/*!
  \fn const T& QGuardedPtr::operator*() const 

  Dereference operator, implements pointer semantics. Just use this
  operator as you would with a normal C++ pointer.
*/

/*!
  \fn QGuardedPtr::operator T*() 

  Cast operator, implements pointer semantics. Because of this function,
  you can pass a QGuardedPtr<X> to a function where an X* is
  required.
*/

/*!
  \fn QGuardedPtr::operator const T*() const

  Cast operator, implements pointer semantics. Because of this function,
  you can pass a QGuardedPtr<X> to a function where a const X* is
  required.
*/

/*!
  \fn QGuardedPtr::operator bool() const

  Cast operator to bool, implements pointer semantics. This function
  allows code such as:
  \code
     QGuardedPtr<QWidget> w = new QLabel;
     ...
     if ( w ) {
	...
     }
  \endcode

  Equivalent to \c !isNull();
*/



/* Internal classes */


QGuardedPtrPrivate::QGuardedPtrPrivate( QObject* o)
    : QObject(0, "_ptrpriv" ), obj( o )
{
    if ( obj )
	connect( obj, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
}


QGuardedPtrPrivate::~QGuardedPtrPrivate()
{
}


void QGuardedPtrPrivate::objectDestroyed()
{
    obj = 0;
}

