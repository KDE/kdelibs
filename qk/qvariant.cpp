/****************************************************************************
** $Id$
**
** Implementation of QVariant class
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

#include "qstring.h"
#include "qfont.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qbrush.h"
#include "qpoint.h"
#include "qrect.h"
#include "qsize.h"
#include "qcolor.h"
#include "qpalette.h"
#include "qiconset.h"
#include "qvariant.h"

// NOT REVISED
/*!
  \class QVariant qvariant.h
  \brief Acts like a union for the most common Qt data types.

  Unfortunately one can not use C++ unions with classes that have
  constructors and destructors since the compiler and the runtime
  library could not determine which destructor to call.

  To come around this QVariant can store the most common Qt and C++
  data types. Like every union it can hold one value of a certain type
  at a time.
*/

/*! \enum QVariant::Type

  This enum type defines the types of variable that a QVariant can
  contain.  Here is a lost of the supported types, and what they
  indicate that the relevant QVariant object contains : <ul>

  <li> \c Invalid - invalid 
  <li> \c String - a QString
  <li> \c StringList - a QStringList
  <li> \c IntList - a QValueList<int>
  <li> \c DoubleList - a QValueList<double>
  <li> \c Font - a QFont
  <li> \c Pixmap - a QPixmap
  <li> \c Brush - a QBrush
  <li> \c Rect - a QRect
  <li> \c Size - a QSize
  <li> \c Color - a QColor
  <li> \c Palette - a QPalette
  <li> \c ColorGroup - a QColorGroup
  <li> \c IconSet - a QIconSet
  <li> \c Point - a QPoint
  <li> \c Image - a QImage
  <li> \c Int - an int
  <li> \c Bool - a bool
  <li> \c Double - a doublea
  <li> \c CString - a QCString

  </ul> Note that Qt's idea of bool depends on the compiler.
  qglobal.h has the system-dependent definition of bool.
*/

/*!
  Creates an invalid variant.
*/
QVariant::QVariant()
{
    typ = Invalid;
}

/*!  Destroys the QVariant and the contained object.

  Note that subclasses that re-implement clear() should reimplement
  the destructor to call clear().  This constructor calls clear(), but
  since it is the destructor, QVariant::clear() is called rather than
  any subclass.
*/
QVariant::~QVariant()
{
    clear();
}

/*!  Constructs a deep copy of the variant passed as argument to this
  constructor.
*/
QVariant::QVariant( const QVariant& p ) : QShared()
{
    typ = Invalid;
    *this = p;
}

/*!
  Reads the variant from the data stream.
*/
QVariant::QVariant( QDataStream& s )
{
    s >> *this;
}

/*!
  Creates a new variant with a string value.
*/
QVariant::QVariant( const QString& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a c-string value.
*/
QVariant::QVariant( const QCString& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a c-string value.
*/
QVariant::QVariant( const char* val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a string list value.
*/
QVariant::QVariant( const QStringList& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a integer list value.
*/
QVariant::QVariant( const QValueList<int>& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a floating point list value.
*/
QVariant::QVariant( const QValueList<double>& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a font value.
*/
QVariant::QVariant( const QFont& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a pixmap value.
*/
QVariant::QVariant( const QPixmap& val )
{
    typ = Invalid;
    setValue( val );
}


/*!
  Creates a new variant with an image value.
*/
QVariant::QVariant( const QImage& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a brush value.
*/
QVariant::QVariant( const QBrush& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a point value.
*/
QVariant::QVariant( const QPoint& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a rect value.
*/
QVariant::QVariant( const QRect& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a size value.
*/
QVariant::QVariant( const QSize& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a color value.
*/
QVariant::QVariant( const QColor& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a color palette value.
*/
QVariant::QVariant( const QPalette& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a color group value.
*/
QVariant::QVariant( const QColorGroup& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with an empty iconset
*/
QVariant::QVariant( const QIconSet& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with an integer value.
*/
QVariant::QVariant( int val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Creates a new variant with a boolean value.
*/
QVariant::QVariant( bool val )
{
    typ = Invalid;
    setValue( val );
}


/*!
  Creates a new variant with a floating point value.
*/
QVariant::QVariant( double val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Assigns the value of one variant to another. This creates a deep copy.
*/
QVariant& QVariant::operator= ( const QVariant& p )
{
    clear();

    switch( p.type() )
	{
	case Invalid:
	    break;
	case String:
	    value.ptr = new QString( p.toString() );
	    break;
	case CString:
	    value.ptr = new QCString( p.toCString() );
	    break;
	case StringList:
	    value.ptr = new QStringList( p.toStringList() );
	    break;
	case IntList:
	    value.ptr = new QValueList<int>( p.toIntList() );
	    break;
	case DoubleList:
	    value.ptr = new QValueList<double>( p.toDoubleList() );
	    break;
	case Font:
	    value.ptr = new QFont( p.toFont() );
	    break;
	case Pixmap:
	    value.ptr = new QPixmap( p.toPixmap() );
	    break;
	case Image:
	    value.ptr = new QImage( p.toImage() );
	    break;
	case Brush:
	    value.ptr = new QBrush( p.toBrush() );
	    break;
	case Point:
	    value.ptr = new QPoint( p.toPoint() );
	    break;
	case Rect:
	    value.ptr = new QRect( p.toRect() );
	    break;
	case Size:
	    value.ptr = new QSize( p.toSize() );
	    break;
	case Color:
	    value.ptr = new QColor( p.toColor() );
	    break;
	case Palette:
	    value.ptr = new QPalette( p.toPalette() );
	    break;
	case ColorGroup:
	    value.ptr = new QColorGroup( p.toColorGroup() );
	    break;
	case IconSet:
	    value.ptr = new QIconSet( p.toIconSet() );
	    break;
	case Int:
	    value.i = p.toInt();
	    break;
	case Bool:
	    value.b = p.toBool();
	    break;
	case Double:
	    value.d = p.toDouble();
	    break;
	default:
	    ASSERT( 0 );
	}

    typ = p.type();

    return *this;
}

/*!
  Returns the type stored in the variant currently in ASCII format.
  The returned strings describe the C++ datatype used to store the
  data, for example "QFont", "QString" or "QValueList<int>".
*/
const char* QVariant::typeName() const
{
    return typeToName( typ );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( const QString& val )
{
    clear();
    typ = String;
    value.ptr = new QString( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( const QCString& val )
{
    clear();
    typ = CString;
    value.ptr = new QCString( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
  The Variant creates a copy of the passed string.
*/
void QVariant::setValue( const char* val )
{
    clear();
    typ = CString;
    value.ptr = new QCString( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
  This function creates a copy of the list. This is very fast since
  QStringList is implicit shared.
*/
void QVariant::setValue( const QStringList& val )
{
    clear();
    typ = StringList;
    value.ptr = new QStringList( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
  This function creates a copy of the list. This is very fast since
  QStringList is implicit shared.
*/
void QVariant::setValue( const QValueList<int>& val )
{
    clear();
    typ = IntList;
    value.ptr = new QValueList<int>( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
  This function creates a copy of the list. This is very fast since
  QStringList is implicit shared.
*/
void QVariant::setValue( const QValueList<double>& val )
{
    clear();
    typ = DoubleList;
    value.ptr = new QValueList<double>( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( const QFont& val )
{
    clear();
    typ = Font;
    value.ptr = new QFont( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( const QPixmap& val )
{
    clear();
    typ = Pixmap;
    value.ptr = new QPixmap( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( const QImage& val )
{
    clear();
    typ = Image;
    value.ptr = new QImage( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( const QBrush& val )
{
    clear();
    typ = Brush;
    value.ptr = new QBrush( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( const QRect& val )
{
    clear();
    typ = Rect;
    value.ptr = new QRect( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( const QPoint& val )
{
    clear();
    typ = Point;
    value.ptr = new QPoint( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( const QSize& val )
{
    clear();
    typ = Size;
    value.ptr = new QSize( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( const QColor& val )
{
    clear();
    typ = Color;
    value.ptr = new QColor( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( const QPalette& val )
{
    clear();
    typ = Palette;
    value.ptr = new QPalette( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( const QColorGroup& val )
{
    clear();
    typ = ColorGroup;
    value.ptr = new QColorGroup( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( const QIconSet& val )
{
    clear();
    typ = IconSet;
    value.ptr = new QIconSet( val );
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( int val )
{
    clear();
    typ = Int;
    value.i = val;
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( bool val )
{
    clear();
    typ = Bool;
    value.b = val;
}

/*!
  Changes the value of this variant. The previous value is dropped.
*/
void QVariant::setValue( double val )
{
    clear();
    typ = Double;
    value.d = val;
}

/*!
  De-allocate, based on the type, producing an Invalid variant.
*/
void QVariant::clear()
{
    switch( typ )
	{
	case String:
	    delete (QString*)value.ptr;
	    break;
	case CString:
	    delete (QCString*)value.ptr;
	    break;
	case IntList:
	    delete (QValueList<int>*)value.ptr;
	    break;
	case DoubleList:
	    delete (QValueList<double>*)value.ptr;
	    break;
	case StringList:
	    delete (QStringList*)value.ptr;
	    break;
	case Font:
	    delete (QFont*)value.ptr;
	    break;
	case Pixmap:
	    delete (QPixmap*)value.ptr;
	    break;
	case Image:
	    delete (QImage*)value.ptr;
	    break;
	case Brush:
	    delete (QBrush*)value.ptr;
	    break;
	case Point:
	    delete (QPoint*)value.ptr;
	    break;
	case Rect:
	    delete (QRect*)value.ptr;
	    break;
	case Size:
	    delete (QSize*)value.ptr;
	    break;
	case Color:
	    delete (QColor*)value.ptr;
	    break;
	case Palette:
	    delete (QPalette*)value.ptr;
	    break;
	case ColorGroup:
	    delete (QColorGroup*)value.ptr;
	    break;
	case IconSet:
	    delete (QIconSet*)value.ptr;
	    break;
	case Invalid:
	case Int:
	case Bool:
	case Double:
	    break;
	default:
	    ASSERT(0);
	}

    typ = Invalid;
}

static const int ntypes = 20;
static const char* type_map[ntypes] =
{
    0,
    "QString",
    "QStringList",
    "QValueList<int>",
    "QValueList<double>",
    "QFont",
    "QPixmap",
    "QBrush",
    "QRect",
    "QSize",
    "QColor",
    "QPalette",
    "QColorGroup",
    "QIconSet",
    "QPoint",
    "QImage",
    "int",
    "bool",
    "double"
    "QCString"
};

/*!
  Converts the enum representation of the storage type to its
  string representation.
*/
const char* QVariant::typeToName( Type typ )
{
    if ( typ >= ntypes )
	return 0;
    return type_map[typ];
}

/*!
  Converts the string representation of the storage type to
  its enum representation.
*/
QVariant::Type QVariant::nameToType( const char* name )
{
    for ( int i = 0; i < ntypes; i++ ) {
	if ( !qstrcmp( type_map[i], name ) )
	    return (Type) i;
    }
    return Invalid;
}

/*!
  Internal function for loading a variant. Use the stream operators
  instead.
*/
void QVariant::load( QDataStream& s )
{
    Q_UINT32 u;
    s >> u;
    Type t = (Type)u;

    switch( t )
	{
	case Invalid:
	    typ = t;
	    break;
	case String:
	    { QString x; s >> x; setValue( x ); }
	    break;
	case CString:
	    { QCString x; s >> x; setValue( x ); }
	    break;
	case StringList:
	    { QStringList x; s >> x; setValue( x ); }
	    break;
	case IntList:
	    { QValueList<int> x; s >> x; setValue( x ); }
	    break;
	case DoubleList:
	    { QValueList<double> x; s >> x; setValue( x ); }
	    break;
	case Font:
	    { QFont x; s >> x; setValue( x ); }
	    break;
	case Pixmap:
	    { QPixmap x; s >> x; setValue( x ); }
	    break;
	case Image:
	    { QImage x; s >> x; setValue( x ); }
	    break;
	case Brush:
	    { QBrush x; s >> x; setValue( x ); }
	    break;
	case Rect:
	    { QRect x; s >> x; setValue( x ); }
	    break;
	case Point:
	    { QPoint x; s >> x; setValue( x ); }
	    break;
	case Size:
	    { QSize x; s >> x; setValue( x ); }
	    break;
	case Color:
	    { QColor x; s >> x; setValue( x ); }
	    break;
	case Palette:
	    { QPalette x; s >> x; setValue( x ); }
	    break;
	case ColorGroup:
	    { QColorGroup x; s >> x; setValue( x ); }
	    break;
	case IconSet:
	    { QPixmap x; s >> x; setValue( QIconSet( x ) ); }
	    break;
	case Int:
	    { int x; s >> x; setValue( x ); };
	    break;
	case Bool:
	    { Q_INT8 x; s >> x; setValue( (bool)x ); };
	    break;
	case Double:
	    { double x; s >> x; setValue( x ); };
	    break;
	default:
	    ASSERT(0);
	}
}

/*!
  Internal function for saving a variant. Use the stream operators
  instead.
*/
void QVariant::save( QDataStream& s ) const
{
    s << (Q_UINT32)type();

    switch( typ )
	{
	case String:
	    s << toString();
	    break;
	case CString:
	    s << toCString();
	    break;
	case StringList:
	    s << toStringList();
	    break;
	case IntList:
	    s << toIntList();
	    break;
	case DoubleList:
	    s << toDoubleList();
	    break;
	case Font:
	    s << toFont();
	    break;
	case Pixmap:
	    s << toPixmap();
	    break;
	case Image:
	    s << toImage();
	    break;
	case Brush:
	    s << toBrush();
	    break;
	case Point:
	    s << toPoint();
	    break;
	case Rect:
	    s << toRect();
	    break;
	case Size:
	    s << toSize();
	    break;
	case Color:
	    s << toColor();
	    break;
	case Palette:
	    s << toPalette();
	    break;
	case ColorGroup:
	    s << toColorGroup();
	    break;
	case IconSet:
	    s << toIconSet().pixmap(); //### add stream operator to iconset #ME
	    break;
	case Int:
	    s << toInt();
	    break;
	case Bool:
	    s << (Q_INT8)toBool();
	    break;
	case Double:
	    s << toDouble();
	    break;
	case Invalid: // fall through
	default:
	    s << QString();
	    break;
	}
}

/*!
  Reads a variant from the stream.
*/
QDataStream& operator>> ( QDataStream& s, QVariant& p )
{
    p.load( s );
    return s;
}

/*!
  Writes a variant to the stream.
*/
QDataStream& operator<< ( QDataStream& s, const QVariant& p )
{
    p.save( s );
    return s;
}

/*!
  Reads a variant type in enum representation from the stream
*/
QDataStream& operator>> ( QDataStream& s, QVariant::Type& p )
{
    Q_UINT32 u;
    s >> u;
    p = (QVariant::Type) u;

    return s;
}

/*!
  Writes a variant type to the stream.
*/
QDataStream& operator<< ( QDataStream& s, const QVariant::Type p )
{
    s << (Q_UINT32)p;

    return s;
}

/*! \fn Type QVariant::type() const
  Returns the stoarge type of the value stored in the
  variant currently.
*/

/*! \fn bool QVariant::isValid() const
  Returns TRUE if the storage type of this variant is QVariant::Invalid.
*/


QString QVariant::toString() const
{
    if ( typ == CString )
	return QString::fromLatin1( toCString() );
    if ( typ != String )
	return QString::null;
    return *((QString*)value.ptr);
}

QCString QVariant::toCString() const
{
    if ( typ == String )
	return toString().latin1();
    if ( typ != CString )
	return 0;
    return *((QCString*)value.ptr);
}

QStringList QVariant::toStringList() const
{
    if ( typ != StringList )
	return QStringList();
    return *((QStringList*)value.ptr);
}

QValueList<int> QVariant::toIntList() const
{
    if ( typ != IntList )
	return QValueList<int>();
    return *((QValueList<int>*)value.ptr);
}

QValueList<double> QVariant::toDoubleList() const
{
    if ( typ != IntList )
	return QValueList<double>();
    return *((QValueList<double>*)value.ptr);
}

QFont QVariant::toFont() const
{
    if ( typ != Font )
	return QFont();
    return *((QFont*)value.ptr);
}

QPixmap QVariant::toPixmap() const
{
    if ( typ != Pixmap )
	return QPixmap();
    return *((QPixmap*)value.ptr);
}

QImage QVariant::toImage() const
{
    if ( typ != Image )
	return QImage();
    return *((QImage*)value.ptr);
}

QBrush QVariant::toBrush() const
{
    if( typ != Brush )
	return QBrush();
    return *((QBrush*)value.ptr);
}

QPoint QVariant::toPoint() const
{
    if ( typ != Point )
	return QPoint();
    return *((QPoint*)value.ptr);
}

QRect QVariant::toRect() const
{
    if ( typ != Rect )
	return QRect();
    return *((QRect*)value.ptr);
}

QSize QVariant::toSize() const
{
    if ( typ != Size )
	return QSize();
    return *((QSize*)value.ptr);
}

QColor QVariant::toColor() const
{
    if ( typ != Color )
	return QColor();
    return *((QColor*)value.ptr);
}

QPalette QVariant::toPalette() const
{
    if ( typ != Palette )
	return QPalette();
    return *((QPalette*)value.ptr);
}

QColorGroup QVariant::toColorGroup() const
{
    if ( typ != ColorGroup )
	return QColorGroup();
    return *((QColorGroup*)value.ptr);
}

QIconSet QVariant::toIconSet() const
{
    if ( typ != IconSet )
	return QIconSet();
    return *((QIconSet*)value.ptr);
}

int QVariant::toInt() const
{
    if( typ != Int )
	return 0;
    return value.i;
}

bool QVariant::toBool() const
{
    if ( typ != Bool )
	return false;
    return value.b;
}

double QVariant::toDouble() const
{
    if ( typ != Double )
	return 0.0;
    return value.d;
}
