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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdcoppropertyproxy.h"
#include "dcopobject.h"

#include <qobject.h>
#include <qmetaobject.h>
#include <qvariant.h>
#include <qcursor.h>
#include <qbitmap.h>
#include <qregion.h>
#include <qpolygon.h>
#include <qicon.h>
#include <qfont.h>
#include <qimage.h>
#include <qbrush.h>
#include <qpalette.h>

#include <ctype.h>
#include <assert.h>

class KDCOPPropertyProxy::Private
{
public:
  Private()
  {
  }
  ~Private()
  {
  }

  QObject *m_object;
};

KDCOPPropertyProxy::KDCOPPropertyProxy( QObject *object ) : d(new Private)
{
  d->m_object = object;
}

KDCOPPropertyProxy::~KDCOPPropertyProxy()
{
  delete d;
}

bool KDCOPPropertyProxy::isPropertyRequest( const DCOPCString &fun )
{
  return isPropertyRequest( fun, d->m_object );
}

bool KDCOPPropertyProxy::processPropertyRequest( const DCOPCString &fun, const QByteArray &data,
                                                 DCOPCString &replyType, QByteArray &replyData )
{
  return processPropertyRequest( fun, data, replyType, replyData, d->m_object );
}

DCOPCStringList KDCOPPropertyProxy::functions()
{
  return functions( d->m_object );
}

bool KDCOPPropertyProxy::isPropertyRequest( const DCOPCString &fun, QObject *object )
{
  if ( fun == "property(QCString)" ||
       fun == "setProperty(QCString,QVariant)" ||
       fun == "propertyNames(bool)" )
    return true;

  bool set;
  DCOPCString propName, arg;
  return decodePropertyRequestInternal( fun, object, set, propName, arg );
}

DCOPCStringList KDCOPPropertyProxy::functions( QObject *object )
{
  DCOPCStringList res;
  res << "QVariant property(QCString property)";
  res << "bool setProperty(QCString name,QVariant property)";
  res << "QValueList<QCString> propertyNames(bool super)";

  const QMetaObject *metaObj = object->metaObject();
  for ( int i = 0; i < metaObj->propertyCount(); i++ ) {
    QMetaProperty metaProp = metaObj->property( i );
	DCOPCString name = metaProp.name();
	name.prepend( " " );
	name.prepend( metaProp.type() );
	name.append( "()" );
	res<<name;
    if ( metaProp.isWritable() )
    {
      DCOPCString setName = metaProp.name();
      setName[ 0 ] = toupper( setName[ 0 ] );
      setName = "void set";
	  setName.append( setName + DCOPCString( "(" ) );
	  setName.append( metaProp.type() );
	  setName.append( DCOPCString( " " ) + metaProp.name() + DCOPCString( ")" ) );
      res << setName;
    }
  }
  return res;

}

bool KDCOPPropertyProxy::processPropertyRequest( const DCOPCString &fun, const QByteArray &data,
                                                 DCOPCString &replyType, QByteArray &replyData,
                                                 QObject *object )
{
  if ( fun == "property(QCString)" )
  {
    DCOPCString propName;
    QDataStream stream( data );
	stream.setVersion(QDataStream::Qt_3_1 );
    stream >> propName;

    replyType = "QVariant";
    QDataStream reply( &replyData, QIODevice::WriteOnly );
	reply.setVersion(QDataStream::Qt_3_1 );
    reply << object->property( propName );
    return true;
  }

  if ( fun == "setProperty(QCString,QVariant)" )
  {
    DCOPCString propName;
    QVariant propValue;
    QDataStream stream( data );
	stream.setVersion(QDataStream::Qt_3_1 );
    stream >> propName >> propValue;

    replyType = "bool";
    QDataStream reply( &replyData, QIODevice::WriteOnly );
    reply << (qint8)object->setProperty( propName, propValue );
	reply.setVersion(QDataStream::Qt_3_1 );
    return true;
  }

  if ( fun == "propertyNames(bool)" )
  {
    qint8 b;
    QDataStream stream( data );
	stream.setVersion(QDataStream::Qt_3_1 );
    stream >> b;

    QList<DCOPCString> res;
	for ( int i = 0 ; i < object->metaObject()->propertyCount(); i++ ) {
		res << object->metaObject()->property( i ).name();
	}

    replyType = "QValueList<QCString>";
    QDataStream reply( &replyData, QIODevice::WriteOnly );
	reply.setVersion(QDataStream::Qt_3_1 );
    reply << res;
    return true;
  }

  bool set;
  DCOPCString propName, arg;

  bool res = decodePropertyRequestInternal( fun, object, set, propName, arg );
  if ( !res )
    return false;

  if ( set )
  {
    QVariant prop;
    QDataStream stream( data );
	stream.setVersion(QDataStream::Qt_3_1 );

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
      DEMARSHAL( Polygon, QPolygon )
      DEMARSHAL( Region, QRegion )
      DEMARSHAL( List, ListType )
      DEMARSHAL( Map, MapType )
      DEMARSHAL( String, QString )
      DEMARSHAL( ByteArray, QByteArray )
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
        bool v;
        stream >> v;
        prop = QVariant( v );
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
	reply.setVersion(QDataStream::Qt_3_1 );

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
      MARSHAL( Polygon )
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
      MARSHAL( Icon )
      MARSHAL2( Int )
      MARSHAL2( UInt )
      case QVariant::Bool:
        reply << (qint8)prop.toBool();
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

bool KDCOPPropertyProxy::decodePropertyRequestInternal( const DCOPCString &fun, QObject *object, bool &set,
                                                        DCOPCString &propName, DCOPCString &arg )
{
  if ( fun.length() < 3 )
    return false;

  set = false;

  propName = fun;

  if ( propName.startsWith( "set" ) )
  {
    propName.detach();
    set = true;
    propName = propName.mid( 3 );
    int p1 = propName.indexOf( '(' );

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
