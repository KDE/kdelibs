/****************************************************************************
** $Id$
**
** Definition of QProperty class
**
** Created : 990414
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

#ifndef QPROPERTY_H
#define QPROPERTY_H

#ifndef QT_H
#include "qstring.h"
#include "qvaluelist.h"
#include "qstringlist.h"
#include "qshared.h"
#include "qdatastream.h"
#endif // QT_H

class QString;
class QFont;
class QPixmap;
// class QMovie;
class QBrush;
class QRect;
class QPoint;
class QImage;
class QSize;
class QColor;
class QPalette;
class QColorGroup;

/**
 * This class acts like a union. It can hold one value at the
 * time but it can hold the most common types.
 * For CORBA people: It is a poor mans CORBA::Any.
 */
class Q_EXPORT QProperty : public QShared
{
public:
    enum Type {
      Empty,
      StringType,
      StringListType,
      IntListType,
      DoubleListType,
      FontType,
      PixmapType,
      BrushType,
      RectType,
      SizeType,
      ColorType,
      PaletteType,
      ColorGroupType,
      PointType,
      ImageType,
      IntType,
      BoolType,
      DoubleType, 
      // MovieType,
      NTypes, 
      CustomType = 0x1000
    };
  
    QProperty();
    QProperty( const QProperty& );
    QProperty( QDataStream& s );
    virtual ~QProperty();
  
    QProperty( const QString& _v ) { setValue( _v ); }
    QProperty( const QStringList& _v ) { setValue( _v ); }
    QProperty( const QValueList<int>& _v ) { setValue( _v ); }
    QProperty( const QValueList<double>& _v ) { setValue( _v ); }
    QProperty( const QFont& _v ) { setValue( _v ); }
    QProperty( const QPixmap& _v ) { setValue( _v ); }
    QProperty( const QImage& _v ) { setValue( _v ); }
  // QProperty( const QMovie& _v ) { setValue( _v ); }
    QProperty( const QBrush& _v ) { setValue( _v ); }
    QProperty( const QPoint& _v ) { setValue( _v ); }
    QProperty( const QRect& _v ) { setValue( _v ); }
    QProperty( const QSize& _v ) { setValue( _v ); }
    QProperty( const QColor& _v ) { setValue( _v ); }
    QProperty( const QPalette& _v ) { setValue( _v ); }
    QProperty( const QColorGroup& _v ) { setValue( _v ); }
    QProperty( int _v ) { setValue( _v ); }
    QProperty( bool _v ) { setValue( _v ); }
    QProperty( double _v ) { setValue( _v ); }
    
    QProperty& operator= ( const QProperty& );
    
    void setValue( const QString& );
    void setValue( const QStringList& );
    void setValue( const QValueList<int>& );
    void setValue( const QValueList<double>& );
    void setValue( const QFont& );
    void setValue( const QPixmap& );
    void setValue( const QImage& );
  // void setValue( const QMovie& );
    void setValue( const QBrush& );
    void setValue( const QPoint& );
    void setValue( const QRect& );
    void setValue( const QSize& );
    void setValue( const QColor& );
    void setValue( const QPalette& );
    void setValue( const QColorGroup& );
    void setValue( int );
    void setValue( bool );
    void setValue( double );

    Type type() const { return typ; }
    virtual QString typeName() const;
  
    bool isEmpty() const { return ( typ == Empty ); }
  
    const QString& stringValue() const { ASSERT( typ == StringType ); return *((QString*)val.ptr); }
    const QStringList& stringListValue() const { ASSERT( typ == StringListType );
                                                 return *((QStringList*)val.ptr); }
    const QValueList<int>& intListValue() const { ASSERT( typ == IntListType );
                                                  return *((QValueList<int>*)val.ptr); }
    const QValueList<double>& doubleListValue() const { ASSERT( typ == IntListType );
                                                        return *((QValueList<double>*)val.ptr); }
    const QFont& fontValue() const { ASSERT( typ == FontType ); return *((QFont*)val.ptr); }
    const QPixmap& pixmapValue() const { ASSERT( typ == PixmapType ); return *((QPixmap*)val.ptr); }
    const QImage& imageValue() const { ASSERT( typ == ImageType ); return *((QImage*)val.ptr); }
  // const QMovie& movieValue() const { ASSERT( typ == MovieType ); return *((QMovie*)val.ptr); }
    const QBrush& brushValue() const { ASSERT( typ == BrushType ); return *((QBrush*)val.ptr); }
    const QPoint& pointValue() const { ASSERT( typ == PointType ); return *((QPoint*)val.ptr); }
    const QRect& rectValue() const { ASSERT( typ == RectType ); return *((QRect*)val.ptr); }
    const QSize& sizeValue() const { ASSERT( typ == SizeType ); return *((QSize*)val.ptr); }
    const QColor& colorValue() const { ASSERT( typ == ColorType ); return *((QColor*)val.ptr); }
    const QPalette& paletteValue() const { ASSERT( typ == PaletteType ); return *((QPalette*)val.ptr); }
    const QColorGroup& colorgroupValue() const { ASSERT( typ == ColorGroupType );
                                               return *((QColorGroup*)val.ptr); }
    int intValue() const { ASSERT( typ == IntType ); return val.i; }
    bool boolValue() const { ASSERT( typ == BoolType ); return val.b; }
    double doubleValue() const { ASSERT( typ == DoubleType ); return val.d; }

    virtual void load( QDataStream& );
    virtual void save( QDataStream& ) const;

    static QString typeToName( Type _typ );
    /**
     * @return QProperty::Empty if the given name is empty or unknown.
     */
    static Type nameToType( const QString& _name );
  
protected:
    virtual void clear();
  
    Type typ;
    union
    {
      int i;
      bool b;
      double d;
      void *ptr;
    } val;

private:
    static void initTypeNameMap();
};

Q_EXPORT QDataStream& operator>> ( QDataStream& s, QProperty& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QProperty& p );
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QProperty::Type& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QProperty::Type p );

#endif

