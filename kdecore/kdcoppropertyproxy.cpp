/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kdcoppropertyproxy.h"

#include <q3strlist.h>
#include <qmetaobject.h>
#include <qvariant.h>
#include <qcursor.h>
#include <qbitmap.h>
#include <qregion.h>
#include <q3pointarray.h>
#include <qicon.h>
#include <qfont.h>
#include <qimage.h>
#include <qbrush.h>
#include <qpalette.h>

#include <ctype.h>
#include <assert.h>

class KDCOPPropertyProxyPrivate
{
public:
  KDCOPPropertyProxyPrivate()
  {
  }
  ~KDCOPPropertyProxyPrivate()
  {
  }

  QObject *m_object;
};

KDCOPPropertyProxy::KDCOPPropertyProxy( QObject *object )
{
  d = new KDCOPPropertyProxyPrivate;
  d->m_object = object;
}

KDCOPPropertyProxy::~KDCOPPropertyProxy()
{
  delete d;
}

bool KDCOPPropertyProxy::isPropertyRequest( const QByteArray &fun )
{
  return isPropertyRequest( fun, d->m_object );
}

bool KDCOPPropertyProxy::processPropertyRequest( const QByteArray &fun, const QByteArray &data,
                                                 QByteArray &replyType, QByteArray &replyData )
{
  return processPropertyRequest( fun, data, replyType, replyData, d->m_object );
}

QList<QByteArray> KDCOPPropertyProxy::functions()
{
  return functions( d->m_object );
}

bool KDCOPPropertyProxy::isPropertyRequest( const QByteArray &fun, QObject *object )
{
  if ( fun == "property(QByteArray)" ||
       fun == "setProperty(QByteArray,QVariant)" ||
       fun == "propertyNames(bool)" )
    return true;

  bool set;
  QByteArray propName, arg;
  return decodePropertyRequestInternal( fun, object, set, propName, arg );
}

QList<QByteArray> KDCOPPropertyProxy::functions( QObject *object )
{
  QList<QByteArray> res;
  res << "QVariant property(QByteArray property)";
  res << "bool setProperty(QByteArray name,QVariant property)";
  res << "QValueList<QByteArray> propertyNames(bool super)";

  const QMetaObject *metaObj = object->metaObject();
  for ( int i = 0; i < metaObj->propertyCount(); i++ ) {
    QMetaProperty metaProp = metaObj->property( i );
	QByteArray name = metaProp.name();
	name.prepend( " " );
	name.prepend( metaProp.type() );
	name.append( "()" );
	res<<name;
    if ( metaProp.isWritable() )
    {
      QByteArray setName = metaProp.name();
      setName[ 0 ] = toupper( setName[ 0 ] );
      setName = "void set";
	  setName.append( setName + QByteArray( "(" ) );
	  setName.append( metaProp.type() );
	  setName.append( QByteArray( " " ) + metaProp.name() + QByteArray( ")" ) );
      res << setName;
    }
  }
  return res;

  //Qt3 code
#if 0
  Q3StrList properties = metaObj->propertyNames( true ); //inherited classes also
  QStrListIterator it( properties );
  for (; it.current(); ++it )
  {
    const QMetaProperty *metaProp = metaObj->property( metaObj->findProperty( it.current(), true ), true );

    assert( metaProp );

    QByteArray name = it.current();
    name.prepend( " " );
    name.prepend( metaProp->type() );
    name.append( "()" );
    res << name;

    if ( metaProp->writable() )
    {
      QByteArray setName = it.current();
      setName[ 0 ] = toupper( setName[ 0 ] );
      setName = "void set" + setName + "(" + metaProp->type() + " " + it.current() + ")";
      res << setName;
    }
  }

  return res;
#endif
}

bool KDCOPPropertyProxy::processPropertyRequest( const QByteArray &fun, const QByteArray &data,
                                                 QByteArray &replyType, QByteArray &replyData,
                                                 QObject *object )
{
  if ( fun == "property(QByteArray)" )
  {
    QByteArray propName;
    QDataStream stream( data );
    stream >> propName;

    replyType = "QVariant";
    QDataStream reply( &replyData, QIODevice::WriteOnly );
    reply << object->property( propName );
    return true;
  }

  if ( fun == "setProperty(QByteArray,QVariant)" )
  {
    QByteArray propName;
    QVariant propValue;
    QDataStream stream( data );
    stream >> propName >> propValue;

    replyType = "bool";
    QDataStream reply( &replyData, QIODevice::WriteOnly );
    reply << (Q_INT8)object->setProperty( propName, propValue );
    return true;
  }

  if ( fun == "propertyNames(bool)" )
  {
    Q_INT8 b;
    QDataStream stream( data );
    stream >> b;

    QList<QByteArray> res;
	for ( int i = 0 ; i < object->metaObject()->propertyCount(); i++ ) {
		res << object->metaObject()->property( i ).name();
	}
	//Qt3 code
#if 0
    Q3StrList props = object->metaObject()->propertyNames( static_cast<bool>( b ) );
    QStrListIterator it( props );
    for (; it.current(); ++it )
      res.append( it.current() );
#endif

    replyType = "QValueList<QByteArray>";
    QDataStream reply( &replyData, QIODevice::WriteOnly );
    reply << res;
    return true;
  }

  bool set;
  QByteArray propName, arg;

  bool res = decodePropertyRequestInternal( fun, object, set, propName, arg );
  if ( !res )
    return false;

  if ( set )
  {
    QVariant prop;
    QDataStream stream( data );

    QVariant::Type type = QVariant::nameToType( arg );
    if ( type == QVariant::Invalid )
      return false;

#define DEMARSHAL( type, val ) \
  case QVariant::type: \
    { \
      val v; \
      stream >> v; \
      prop = QVariant( v ); \
    } \
    break;

    typedef QList<QVariant> ListType;
    typedef QMap<QString,QVariant> MapType;

    switch ( type )
    {
      DEMARSHAL( Cursor, QCursor )
      DEMARSHAL( Bitmap, QBitmap )
      DEMARSHAL( PointArray, Q3PointArray )
      DEMARSHAL( Region, QRegion )
      DEMARSHAL( List, ListType )
      DEMARSHAL( Map, MapType )
      DEMARSHAL( String, QString )
      DEMARSHAL( CString, QByteArray )
      DEMARSHAL( StringList, QStringList )
      DEMARSHAL( Font, QFont )
      DEMARSHAL( Pixmap, QPixmap )
      DEMARSHAL( Image, QImage )
      DEMARSHAL( Brush, QBrush )
      DEMARSHAL( Point, QPoint )
      DEMARSHAL( Rect, QRect )
      DEMARSHAL( Size, QSize )
      DEMARSHAL( Color, QColor )
      DEMARSHAL( Palette, QPalette )
      DEMARSHAL( ColorGroup, QColorGroup )
      case QVariant::Icon:
      {
        QPixmap val;
        stream >> val;
        prop = QVariant( QIcon( val ) );
      }
      break;
      DEMARSHAL( Int, int )
      DEMARSHAL( UInt, uint )
      case QVariant::Bool:
      {
        Q_INT8 v;
        stream >> v;
        prop = QVariant( static_cast<bool>( v ), 1 );
      }
        break;
      DEMARSHAL( Double, double )
      default:
        return false;
    }

    replyType = "void";
    return object->setProperty( propName, prop );
  }
  else
  {
    QVariant prop = object->property( propName );

    if ( prop.type() == QVariant::Invalid )
      return false;

    replyType = prop.typeName();
    QDataStream reply( &replyData, QIODevice::WriteOnly );

#define MARSHAL( type ) \
  case QVariant::type: \
    reply << qvariant_cast<Q##type>(prop); \
    break;

#define MARSHAL2( type ) \
  case QVariant::type: \
    reply << prop.to##type (); \
    break;

    switch ( prop.type() )
    {
      MARSHAL( Cursor )
      MARSHAL( Bitmap )
//      MARSHAL( PointArray )
      MARSHAL( Region )
      MARSHAL2( List )
      MARSHAL2( Map )
      MARSHAL( String )
      MARSHAL( ByteArray )
      MARSHAL( StringList )
      MARSHAL( Font )
      MARSHAL( Pixmap )
      MARSHAL( Image )
      MARSHAL( Brush )
      MARSHAL( Point )
      MARSHAL( Rect )
      MARSHAL( Size )
      MARSHAL( Color )
      MARSHAL( Palette )
      MARSHAL( ColorGroup )
      MARSHAL( Icon )
      MARSHAL2( Int )
      MARSHAL2( UInt )
      case QVariant::Bool:
        reply << (Q_INT8)prop.toBool();
        break;
      MARSHAL2( Double )
      default:
        return false;
    }

#undef MARSHAL
#undef DEMARSHAL

    return true;
  }

  return false;
}

bool KDCOPPropertyProxy::decodePropertyRequestInternal( const QByteArray &fun, QObject *object, bool &set,
                                                        QByteArray &propName, QByteArray &arg )
{
  if ( fun.length() < 3 )
    return false;

  set = false;

  propName = fun;

  if ( propName.left( 3 ) == "set" )
  {
    propName.detach();
    set = true;
    propName = propName.mid( 3 );
    int p1 = propName.find( '(' );

    uint len = propName.length();

    if ( propName[ len - 1 ] != ')' )
      return false;

    arg = propName.mid( p1+1, len - p1 - 2 );
    propName.truncate( p1 );
    propName[ 0 ] = tolower( propName[ 0 ] );
  }
  else
    propName.truncate( propName.length() - 2 );

  if ( object->metaObject()->indexOfProperty( propName ) == -1 )
    return false;

  return true;
}
