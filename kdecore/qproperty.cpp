/****************************************************************************
** $Id$
**
** Implementation of QProperty class
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

#include <qstring.h>
#include <qfont.h>
#include <qpixmap.h>
#include <qimage.h>
// #include <qmovie.h>
#include <qbrush.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qcolor.h>
#include <qpalette.h>

#include "qproperty.h"

/*!
  \class QProperty qproperty.h
  \brief Acts like a union for the most common Qt data types.

  Unfortunately one can not use C++ unions with classes that have
  constructors and destructors since the compiler and the runtime
  library could not determine which destructor to call.

  To come around this QProperty can store the most common Qt and C++
  data types. Like every union it can hold one value of a certain type
  at a time.
*/

/*!
  Creates an empty property.
*/
QProperty::QProperty()
{
  typ = Empty;
}

/*!
  Subclasses which re-implement clear() should reimplement the
  destructor to call clear() - so that the overridden clear() is
  called.
*/
QProperty::~QProperty()
{
    clear();
}

/*!
  Constructs a deep copy of the property passed as argument to this constructor.
*/
QProperty::QProperty( const QProperty& p ) : QShared()
{
  typ = Empty;
  *this = p;
}

/*!
  Reads the property from the data stream.
*/
QProperty::QProperty( QDataStream& s )
{
  s >> *this;
}

/*!
  Assigns the value of one property to another. This creates a deep copy.
*/
QProperty& QProperty::operator= ( const QProperty& p )
{
  clear();

  switch( p.type() )
    {
    case Empty:
      break;
    case StringType:
      val.ptr = new QString( p.stringValue() );
      break;
    case StringListType:
      val.ptr = new QStringList( p.stringListValue() );
      break;
    case IntListType:
      val.ptr = new QValueList<int>( p.intListValue() );
      break;
    case DoubleListType:
      val.ptr = new QValueList<double>( p.doubleListValue() );
      break;
    case FontType:
      val.ptr = new QFont( p.fontValue() );
      break;
      // case MovieType:
      // val.ptr = new QMovie( p.movieValue() );
      // break;
    case PixmapType:
      val.ptr = new QPixmap( p.pixmapValue() );
      break;
    case ImageType:
      val.ptr = new QImage( p.imageValue() );
      break;
    case BrushType:
      val.ptr = new QBrush( p.brushValue() );
      break;
    case PointType:
      val.ptr = new QPoint( p.pointValue() );
      break;
    case RectType:
      val.ptr = new QRect( p.rectValue() );
      break;
    case SizeType:
      val.ptr = new QSize( p.sizeValue() );
      break;
    case ColorType:
      val.ptr = new QColor( p.colorValue() );
      break;
    case PaletteType:
      val.ptr = new QPalette( p.paletteValue() );
      break;
    case ColorGroupType:
      val.ptr = new QColorGroup( p.colorgroupValue() );
      break;
    case IntType:
      val.i = p.intValue();
      break;
    case BoolType:
      val.b = p.boolValue();
      break;
    case DoubleType:
      val.d = p.doubleValue();
      break;
    default:
      ASSERT( 0 );
    }

  typ = p.type();

  return *this;
}

/*!
  Returns the type stored in the property currently in ASCII format.
  The returned strings describe the C++ datatype used to store the
  data, for example "QFont", "QString" or "QValueList<int>".
*/
QString QProperty::typeName() const
{
  return typeToName( typ );
}

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( const QString& _value )
{
  clear();
  typ = StringType;
  val.ptr = new QString( _value );
}

/*!
  Changes the value of this property. The previous value is dropped.
  This function creates a copy of the list. This is very fast since
  QStringList is implicit shared.
*/
void QProperty::setValue( const QStringList& _value )
{
  clear();
  typ = StringListType;
  val.ptr = new QStringList( _value );
}

/*!
  Changes the value of this property. The previous value is dropped.
  This function creates a copy of the list. This is very fast since
  QStringList is implicit shared.
*/
void QProperty::setValue( const QValueList<int>& _value )
{
  clear();
  typ = IntListType;
  val.ptr = new QValueList<int>( _value );
}

/*!
  Changes the value of this property. The previous value is dropped.
  This function creates a copy of the list. This is very fast since
  QStringList is implicit shared.
*/
void QProperty::setValue( const QValueList<double>& _value )
{
  clear();
  typ = DoubleListType;
  val.ptr = new QValueList<double>( _value );
}

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( const QFont& _value )
{
  clear();
  typ = FontType;
  val.ptr = new QFont( _value );
}

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( const QPixmap& _value )
{
  clear();
  typ = PixmapType;
  val.ptr = new QPixmap( _value );
}

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( const QImage& _value )
{
  clear();
  typ = ImageType;
  val.ptr = new QImage( _value );
}

// void QProperty::setValue( const QMovie& _value )
// {
//   clear();
//   typ = MovieType;
//  val.ptr = new QMovie( _value );
// }

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( const QBrush& _value )
{
  clear();
  typ = BrushType;
  val.ptr = new QBrush( _value );
}

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( const QRect& _value )
{
  clear();
  typ = RectType;
  val.ptr = new QRect( _value );
}

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( const QPoint& _value )
{
  clear();
  typ = PointType;
  val.ptr = new QPoint( _value );
}

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( const QSize& _value )
{
  clear();
  typ = SizeType;
  val.ptr = new QSize( _value );
}

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( const QColor& _value )
{
  clear();
  typ = ColorType;
  val.ptr = new QColor( _value );
}

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( const QPalette& _value )
{
  clear();
  typ = PaletteType;
  val.ptr = new QPalette( _value );
}

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( const QColorGroup& _value )
{
  clear();
  typ = ColorGroupType;
  val.ptr = new QColorGroup( _value );
}

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( int _value )
{
  clear();
  typ = IntType;
  val.i = _value;
}

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( bool _value )
{
  clear();
  typ = BoolType;
  val.b = _value;
}

/*!
  Changes the value of this property. The previous value is dropped.
*/
void QProperty::setValue( double _value )
{
  clear();
  typ = DoubleType;
  val.d = _value;
}

/*!
  De-allocate, based on the type, producing an Empty property.
*/
void QProperty::clear()
{
  switch( typ )
    {
    case Empty:
    case IntType:
    case BoolType:
    case DoubleType:
      break;
    case StringType:
      delete (QString*)val.ptr;
      break;
    case IntListType:
      delete (QValueList<int>*)val.ptr;
      break;
    case DoubleListType:
      delete (QValueList<double>*)val.ptr;
      break;
    case StringListType:
      delete (QStringList*)val.ptr;
      break;
    case FontType:
      delete (QFont*)val.ptr;
      break;
      // case MovieType:
      // delete (QMovie*)val.ptr;
      // break;
    case PixmapType:
      delete (QPixmap*)val.ptr;
      break;
    case ImageType:
      delete (QImage*)val.ptr;
      break;
    case BrushType:
      delete (QBrush*)val.ptr;
      break;
    case PointType:
      delete (QPoint*)val.ptr;
      break;
    case RectType:
      delete (QRect*)val.ptr;
      break;
    case SizeType:
      delete (QSize*)val.ptr;
      break;
    case ColorType:
      delete (QColor*)val.ptr;
      break;
    case PaletteType:
      delete (QPalette*)val.ptr;
      break;
    case ColorGroupType:
      delete (QColorGroup*)val.ptr;
      break;
    default:
      ASSERT( 0 );
    }

  typ = Empty;
}

static QString *typ_to_name = 0;

void QProperty::initTypeNameMap()
{
    if ( typ_to_name ) return;

    typ_to_name = new QString[NTypes];

    typ_to_name[(int)Empty] = QString::null;
    typ_to_name[(int)StringType] = QString::fromLatin1("QString");
    typ_to_name[(int)StringListType] = QString::fromLatin1("QStringList");
    typ_to_name[(int)IntListType] = QString::fromLatin1("QValueList<int>");
    typ_to_name[(int)DoubleListType] = QString::fromLatin1("QValueList<double>");
    typ_to_name[(int)FontType] = QString::fromLatin1("QFont");
    typ_to_name[(int)PixmapType] = QString::fromLatin1("QPixmap");
    typ_to_name[(int)ImageType] = QString::fromLatin1("QImage");
    typ_to_name[(int)BrushType] = QString::fromLatin1("QBrush");
    typ_to_name[(int)RectType] = QString::fromLatin1("QRect");
    typ_to_name[(int)PointType] = QString::fromLatin1("QPoint");
    typ_to_name[(int)SizeType] = QString::fromLatin1("QSize");
    typ_to_name[(int)ColorType] = QString::fromLatin1("QColor");
    typ_to_name[(int)PaletteType] = QString::fromLatin1("QPalette");
    typ_to_name[(int)ColorGroupType] = QString::fromLatin1("QColorGroup");
    typ_to_name[(int)IntType] = QString::fromLatin1("int");
    typ_to_name[(int)BoolType] = QString::fromLatin1("bool");
    typ_to_name[(int)DoubleType] = QString::fromLatin1("double");
    //typ_to_name[(int)MovieType] = QString::fromLatin1("QMovie");
}

/*!
  Converts the enum representation of the storage type to its
  string representation.
*/
QString QProperty::typeToName( QProperty::Type _typ )
{
    initTypeNameMap();
    return typ_to_name[_typ];
}

/*!
  Converts the string representation of the storage type to
  its enum representation.
*/
QProperty::Type QProperty::nameToType( const QString& _name )
{
    initTypeNameMap();
    int t = (int)NTypes;
    while ( t > (int)Empty && typ_to_name[(int)--t] != _name )
	;
    return Type(t);
}

/*!
  Internal function for loading a property. Use the stream operators
  instead.
*/
void QProperty::load( QDataStream& s )
{
  Q_UINT32 u;
  s >> u;
  Type t = (Type)u;

  switch( t )
    {
    case Empty:
      typ = t;
      break;
    case StringType:
      { QString x; s >> x; setValue( x ); }
      break;
    case StringListType:
      { QStringList x; s >> x; setValue( x ); }
      break;
    case IntListType:
      { QValueList<int> x; s >> x; setValue( x ); }
      break;
    case DoubleListType:
      { QValueList<double> x; s >> x; setValue( x ); }
      break;
    case FontType:
      { QFont x; s >> x; setValue( x ); }
      break;
      // case MovieType:
      // return "QMovie";
    case PixmapType:
      { QPixmap x; s >> x; setValue( x ); }
      break;
    case ImageType:
      { QImage x; s >> x; setValue( x ); }
      break;
    case BrushType:
      { QBrush x; s >> x; setValue( x ); }
      break;
    case RectType:
      { QRect x; s >> x; setValue( x ); }
      break;
    case PointType:
      { QPoint x; s >> x; setValue( x ); }
      break;
    case SizeType:
      { QSize x; s >> x; setValue( x ); }
      break;
    case ColorType:
      { QColor x; s >> x; setValue( x ); }
      break;
    case PaletteType:
      { QPalette x; s >> x; setValue( x ); }
      break;
    case ColorGroupType:
      { QColorGroup x; s >> x; setValue( x ); }
      break;
    case IntType:
      { int x; s >> x; setValue( x ); };
      break;
    case BoolType:
      { Q_INT8 x; s >> x; setValue( (bool)x ); };
      break;
    case DoubleType:
      { double x; s >> x; setValue( x ); };
      break;
    default:
      ASSERT( 0 );
    }
}

/*!
  Internal function for saving a property. Use the stream operators
  instead.
*/
void QProperty::save( QDataStream& s ) const
{
  s << (Q_UINT32)type();

  switch( typ )
    {
    case Empty:
      s << QString();
      break;
    case StringType:
      s << stringValue();
      break;
    case StringListType:
      s << stringListValue();
      break;
    case IntListType:
      s << intListValue();
      break;
    case DoubleListType:
      s << doubleListValue();
      break;
    case FontType:
      s << fontValue();
      break;
      // case MovieType:
      // return "QMovie";
    case PixmapType:
      s << pixmapValue();
      break;
    case ImageType:
      s << imageValue();
      break;
    case BrushType:
      s << brushValue();
      break;
    case PointType:
      s << pointValue();
      break;
    case RectType:
      s << rectValue();
      break;
    case SizeType:
      s << sizeValue();
      break;
    case ColorType:
      s << colorValue();
      break;
    case PaletteType:
      s << paletteValue();
      break;
    case ColorGroupType:
      s << colorgroupValue();
      break;
    case IntType:
      s << intValue();
      break;
    case BoolType:
      s << (Q_INT8)boolValue();
      break;
    case DoubleType:
      s << doubleValue();
      break;
    default:
      ASSERT( 0 );
    }
}

/*!
  Reads a property from the stream.
*/
QDataStream& operator>> ( QDataStream& s, QProperty& p )
{
  p.load( s );
  return s;
}

/*!
  Writes a property to the stream.
*/
QDataStream& operator<< ( QDataStream& s, const QProperty& p )
{
  p.save( s );
  return s;
}

/*!
  Reads a property type in enum representation from the stream
*/
QDataStream& operator>> ( QDataStream& s, QProperty::Type& p )
{
  Q_UINT32 u;
  s >> u;
  p = (QProperty::Type) u;

  return s;
}

/*!
  Writes a property type to the stream.
*/
QDataStream& operator<< ( QDataStream& s, const QProperty::Type p )
{
  s << (Q_UINT32)p;

  return s;
}


/*! \fn QProperty::QProperty( const QString& _v )
  Creates a new property with a string value.
*/

/*! \fn QProperty::QProperty( const QStringList& _v )
  Creates a new property with a string list value.
*/

/*! \fn QProperty::QProperty( const QValueList<int>& _v )
  Creates a new property with a integer list value.
*/

/*! \fn QProperty::QProperty( const QValueList<double>& _v )
  Creates a new property with a floating point list value.
*/

/*! \fn QProperty::QProperty( const QFont& _v )
  Creates a new property with a font value.
*/

/*! \fn QProperty::QProperty( const QPixmap& _v )
  Creates a new property with a pixmap value.
*/

/*! \fn QProperty::QProperty( const QImage& _v )
  Creates a new property with an image value.
*/

/*! \fn QProperty::QProperty( const QBrush& _v )
  Creates a new property with a brush value.
*/

/*! \fn QProperty::QProperty( const QPoint& _v )
  Creates a new property with a point value.
*/

/*! \fn QProperty::QProperty( const QRect& _v )
  Creates a new property with a rect value.
*/

/*! \fn QProperty::QProperty( const QSize& _v )
  Creates a new property with a size value.
*/

/*! \fn QProperty::QProperty( const QColor& _v )
  Creates a new property with a color value.
*/

/*! \fn QProperty::QProperty( const QPalette& _v )
  Creates a new property with a color palette value.
*/

/*! \fn QProperty::QProperty( const QColorGroup& _v )
  Creates a new property with a color group value.
*/

/*! \fn QProperty::QProperty( int _v )
  Creates a new property with an integer value.
*/

/*! \fn QProperty::QProperty( bool _v )
  Creates a new property with a boolean value.
*/

/*! \fn QProperty::QProperty( double _v )
  Creates a new property with a floating point value.
*/

/*! \fn Type QProperty::type() const
  Returns the stoarge type of the value stored in the
  property currently.
*/

/*! \fn bool QProperty::isEmpty() const
  Returns TRUE if the storage type of this property is QProperty::Empty.
*/

/*! \fn const QString& QProperty::stringValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn const QStringList& QProperty::stringListValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn const QValueList<int>& QProperty::intListValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn const QValueList<double>& QProperty::doubleListValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn const QFont& QProperty::fontValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn const QPixmap& QProperty::pixmapValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn const QImage& QProperty::imageValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn const QBrush& QProperty::brushValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn const QPoint& QProperty::pointValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn const QRect& QProperty::rectValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn const QSize& QProperty::sizeValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn const QColor& QProperty::colorValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn const QPalette& QProperty::paletteValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn const QColorGroup& QProperty::colorgroupValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn int QProperty::intValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn bool QProperty::boolValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/

/*! \fn double QProperty::doubleValue() const
  Returns the value stored in the property. If the properties
  value does not match the return type of this function then
  this function will abort your process. So check with type()
  or typeName() first wether the property holds the correct
  data type.
*/
