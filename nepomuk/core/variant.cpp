/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "variant.h"
#include "resource.h"


Nepomuk::KMetaData::Variant::Variant()
  : QVariant()
{
}


Nepomuk::KMetaData::Variant::~Variant()
{
}


Nepomuk::KMetaData::Variant::Variant( Type type )
  : QVariant( type )
{
}


Nepomuk::KMetaData::Variant::Variant( int typeOrUserType, const void *copy )
  : QVariant( typeOrUserType, copy )
{
}


Nepomuk::KMetaData::Variant::Variant( const QVariant& other )
  : QVariant( other )
{
}


Nepomuk::KMetaData::Variant::Variant( int i )
  : QVariant( i )
{
}


Nepomuk::KMetaData::Variant::Variant( uint ui )
  : QVariant( ui )
{
}


Nepomuk::KMetaData::Variant::Variant( qlonglong ll )
  : QVariant( ll )
{
}


Nepomuk::KMetaData::Variant::Variant( qulonglong ull )
  : QVariant( ull )
{
}


Nepomuk::KMetaData::Variant::Variant( bool b )
  : QVariant( b )
{
}


Nepomuk::KMetaData::Variant::Variant( double d )
  : QVariant( d )
{
}


#ifndef QT_NO_CAST_FROM_ASCII
Nepomuk::KMetaData::Variant::Variant( const char *str )
  : QVariant( str )
{
}
#endif


Nepomuk::KMetaData::Variant::Variant( const QByteArray& bytearray )
  : QVariant( bytearray )
{
}


Nepomuk::KMetaData::Variant::Variant( const QBitArray& bitarray )
  : QVariant( bitarray )
{
}


Nepomuk::KMetaData::Variant::Variant( const QString& string )
  : QVariant( string )
{
}


Nepomuk::KMetaData::Variant::Variant( const QLatin1String& string )
  : QVariant( string )
{
}


Nepomuk::KMetaData::Variant::Variant( const QStringList& stringlist )
  : QVariant( stringlist )
{
}


Nepomuk::KMetaData::Variant::Variant( const QChar& qchar )
  : QVariant( qchar )
{
}


Nepomuk::KMetaData::Variant::Variant( const QDate& date )
  : QVariant( date )
{
}


Nepomuk::KMetaData::Variant::Variant( const QTime& time )
  : QVariant( time )
{
}


Nepomuk::KMetaData::Variant::Variant( const QDateTime& datetime )
  : QVariant( datetime )
{
}


Nepomuk::KMetaData::Variant::Variant( const QList<QVariant>& list )
  : QVariant( list )
{
}


Nepomuk::KMetaData::Variant::Variant( const QMap<QString,QVariant>& map )
  : QVariant( map )
{
}


#ifndef QT_NO_GEOM_VARIANT
Nepomuk::KMetaData::Variant::Variant( const QSize& size )
  : QVariant( size )
{
}


Nepomuk::KMetaData::Variant::Variant( const QSizeF& size )
  : QVariant( size )
{
}


Nepomuk::KMetaData::Variant::Variant( const QPoint& pt )
  : QVariant( pt )
{
}


Nepomuk::KMetaData::Variant::Variant( const QPointF& pt )
  : QVariant( pt )
{
}


Nepomuk::KMetaData::Variant::Variant( const QLine& line )
  : QVariant( line )
{
}


Nepomuk::KMetaData::Variant::Variant( const QLineF& line )
  : QVariant( line )
{
}


Nepomuk::KMetaData::Variant::Variant( const QRect& rect )
  : QVariant( rect )
{
}


Nepomuk::KMetaData::Variant::Variant( const QRectF& rect )
  : QVariant( rect )
{
}


#endif
Nepomuk::KMetaData::Variant::Variant( const QUrl& url )
  : QVariant( url )
{
}


Nepomuk::KMetaData::Variant::Variant( const QLocale& locale )
  : QVariant( locale )
{
}


Nepomuk::KMetaData::Variant::Variant( const QRegExp& regExp )
  : QVariant( regExp )
{
}


Nepomuk::KMetaData::Variant::Variant( Qt::GlobalColor color )
  : QVariant( color )
{
}



Nepomuk::KMetaData::Variant::Variant( const Nepomuk::KMetaData::Resource& r )
  : QVariant()
{
  setValue( r );
}


Nepomuk::KMetaData::Resource Nepomuk::KMetaData::Variant::toResource() const
{
  return value<Resource>();
}
