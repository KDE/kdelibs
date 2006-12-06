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


Nepomuk::KMetaData::Variant::Variant( const Variant& other )
  : QVariant( other )
{
}


Nepomuk::KMetaData::Variant::Variant( int i )
  : QVariant( i )
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


Nepomuk::KMetaData::Variant::Variant( const QString& string )
  : QVariant( string )
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


Nepomuk::KMetaData::Variant::Variant( const QUrl& url )
  : QVariant( url )
{
}


Nepomuk::KMetaData::Variant::Variant( const Nepomuk::KMetaData::Resource& r )
  : QVariant()
{
  setValue( r );
}


Nepomuk::KMetaData::Variant::Variant( const QList<int>& i )
  : QVariant()
{
  setValue( i );
}


Nepomuk::KMetaData::Variant::Variant( const QList<bool>& b )
  : QVariant()
{
  setValue( b );
}


Nepomuk::KMetaData::Variant::Variant( const QList<double>& d )
  : QVariant()
{
  setValue( d );
}


Nepomuk::KMetaData::Variant::Variant( const QStringList& stringlist )
  : QVariant( stringlist )
{
}


Nepomuk::KMetaData::Variant::Variant( const QList<QDate>& date )
  : QVariant()
{
  setValue( date );
}


Nepomuk::KMetaData::Variant::Variant( const QList<QTime>& time )
  : QVariant()
{
  setValue( time );
}


Nepomuk::KMetaData::Variant::Variant( const QList<QDateTime>& datetime )
  : QVariant()
{
  setValue( datetime );
}


Nepomuk::KMetaData::Variant::Variant( const QList<QUrl>& url )
  : QVariant()
{
  setValue( url );
}



Nepomuk::KMetaData::Variant::Variant( const QList<Resource>& r )
  : QVariant()
{
  setValue( r );
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const Variant& v )
{
  QVariant::operator=( v );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( int i )
{
  setValue( i );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( bool b )
{
  setValue( b );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( double d )
{
  setValue( d );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QString& string )
{
  setValue( string );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QDate& date )
{
  setValue( date );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QTime& time )
{
  setValue( time );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QDateTime& datetime )
{
  setValue( datetime );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QUrl& url )
{
  setValue( url );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const Resource& r )
{
  setValue( r );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<int>& i )
{
  setValue( i );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<bool>& b )
{
  setValue( b );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<double>& d )
{
  setValue( d );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QStringList& stringlist )
{
  setValue( stringlist );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<QDate>& date )
{
  setValue( date );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<QTime>& time )
{
  setValue( time );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<QDateTime>& datetime )
{
  setValue( datetime );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<QUrl>& url )
{
  setValue( url );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<Resource>& r )
{
  setValue( r );
  return *this;
}


void Nepomuk::KMetaData::Variant::append( int i )
{
  QList<int> l = toIntList();
  l.append( i );
  operator=( l );
}


void Nepomuk::KMetaData::Variant::append( bool b )
{
  QList<bool> l = toBoolList();
  l.append( b );
  operator=( l );
}


void Nepomuk::KMetaData::Variant::append( double d )
{
  QList<double> l = toDoubleList();
  l.append( d );
  operator=( l );
}


void Nepomuk::KMetaData::Variant::append( const QString& string )
{
  QStringList l = toStringList();
  l.append( string );
  operator=( l );
}


void Nepomuk::KMetaData::Variant::append( const QDate& date )
{
  QList<QDate> l = toDateList();
  l.append( date );
  operator=( l );
}


void Nepomuk::KMetaData::Variant::append( const QTime& time )
{
  QList<QTime> l = toTimeList();
  l.append( time );
  operator=( l );
}


void Nepomuk::KMetaData::Variant::append( const QDateTime& datetime )
{
  QList<QDateTime> l = toDateTimeList();
  l.append( datetime );
  operator=( l );
}


void Nepomuk::KMetaData::Variant::append( const QUrl& url )
{
  QList<QUrl> l = toUrlList();
  l.append( url );
  operator=( l );
}


void Nepomuk::KMetaData::Variant::append( const Resource& r )
{
  QList<Resource> l = toResourceList();
  l.append( r );
  operator=( l );
}


void Nepomuk::KMetaData::Variant::append( const Variant& v )
{
  if( v.simpleType() == qMetaTypeId<int>() ) {
    operator=( toIntList() += v.toIntList() );
  }
  else if( v.simpleType() == qMetaTypeId<bool>() ) {
    operator=( toBoolList() += v.toBoolList() );
  }
  else if( v.simpleType() == qMetaTypeId<double>() ) {
    operator=( toDoubleList() += v.toDoubleList() );
  }
  else if( v.simpleType() == qMetaTypeId<QString>() ) {
    operator=( toStringList() += v.toStringList() );
  }
  else if( v.simpleType() == qMetaTypeId<QDate>() ) {
    operator=( toDateList() += v.toDateList() );
  }
  else if( v.simpleType() == qMetaTypeId<QTime>() ) {
    operator=( toTimeList() += v.toTimeList() );
  }
  else if( v.simpleType() == qMetaTypeId<QDateTime>() ) {
    operator=( toDateTimeList() += v.toDateTimeList() );
  }
  else if( v.simpleType() == qMetaTypeId<QUrl>() ) {
    operator=( toUrlList() += v.toUrlList() );
  }
  else if( v.simpleType() == qMetaTypeId<Resource>() ) {
    operator=( toResourceList() += v.toResourceList() );
  }
  else
    qDebug() << "(Variant::append) unknown type: " << v.simpleType() << endl;
}


bool Nepomuk::KMetaData::Variant::isInt() const
{
  return( type() == qMetaTypeId<int>() );
}


bool Nepomuk::KMetaData::Variant::isBool() const
{
  return( type() == qMetaTypeId<bool>() );
}


bool Nepomuk::KMetaData::Variant::isDouble() const
{
  return( type() == qMetaTypeId<double>() );
}


bool Nepomuk::KMetaData::Variant::isString() const
{
  return( type() == qMetaTypeId<QString>() );
}


bool Nepomuk::KMetaData::Variant::isDate() const
{
  return( type() == qMetaTypeId<QDate>() );
}


bool Nepomuk::KMetaData::Variant::isTime() const
{
  return( type() == qMetaTypeId<QTime>() );
}


bool Nepomuk::KMetaData::Variant::isDateTime() const
{
  return( type() == qMetaTypeId<QDateTime>() );
}


bool Nepomuk::KMetaData::Variant::isUrl() const
{
  return( type() == qMetaTypeId<QUrl>() );
}


bool Nepomuk::KMetaData::Variant::isResource() const
{
  return( type() == qMetaTypeId<Resource>() );
}


bool Nepomuk::KMetaData::Variant::isIntList() const
{
  return( type() == qMetaTypeId<QList<int> >() );
}


bool Nepomuk::KMetaData::Variant::isBoolList() const
{
  return( type() == qMetaTypeId<QList<bool> >() );
}


bool Nepomuk::KMetaData::Variant::isDoubleList() const
{
  return( type() == qMetaTypeId<QList<double> >() );
}


bool Nepomuk::KMetaData::Variant::isStringList() const
{
  return( type() == qMetaTypeId<QStringList>() );
}


bool Nepomuk::KMetaData::Variant::isDateList() const
{
  return( type() == qMetaTypeId<QList<QDate> >() );
}


bool Nepomuk::KMetaData::Variant::isTimeList() const
{
  return( type() == qMetaTypeId<QList<QTime> >() );
}


bool Nepomuk::KMetaData::Variant::isDateTimeList() const
{
  return( type() == qMetaTypeId<QList<QDateTime> >() );
}


bool Nepomuk::KMetaData::Variant::isUrlList() const
{
  return( type() == qMetaTypeId<QList<QUrl> >() );
}


bool Nepomuk::KMetaData::Variant::isResourceList() const
{
  return( type() == qMetaTypeId<QList<Resource> >() );
}



int Nepomuk::KMetaData::Variant::toInt() const
{
  return value<int>();
}


bool Nepomuk::KMetaData::Variant::toBool() const
{
  return value<bool>();
}


double Nepomuk::KMetaData::Variant::toDouble() const
{
  return value<double>();
}


QString Nepomuk::KMetaData::Variant::toString() const
{
  return value<QString>();
}


QDate Nepomuk::KMetaData::Variant::toDate() const
{
  return value<QDate>();
}


QTime Nepomuk::KMetaData::Variant::toTime() const
{
  return value<QTime>();
}


QDateTime Nepomuk::KMetaData::Variant::toDateTime() const
{
  return value<QDateTime>();
}


QUrl Nepomuk::KMetaData::Variant::toUrl() const
{
  return value<QUrl>();
}


Nepomuk::KMetaData::Resource Nepomuk::KMetaData::Variant::toResource() const
{
  return value<Resource>();
}



QList<int> Nepomuk::KMetaData::Variant::toIntList() const
{
  if( isInt() ) {
    QList<int> l;
    l.append( toInt() );
    return l;
  }
  else
    return value<QList<int> >();
}


QList<bool> Nepomuk::KMetaData::Variant::toBoolList() const
{
  if( isBool() ) {
    QList<bool> l;
    l.append( toBool() );
    return l;
  }
  else
    return value<QList<bool> >();
}


QList<double> Nepomuk::KMetaData::Variant::toDoubleList() const
{
  if( isDouble() ) {
    QList<double> l;
    l.append( toDouble() );
    return l;
  }
  else
    return value<QList<double> >();
}


QStringList Nepomuk::KMetaData::Variant::toStringList() const
{
  if( isString() )
    return QStringList( toString() );
  else
    return value<QStringList>();
}


QList<QDate> Nepomuk::KMetaData::Variant::toDateList() const
{
  if( isDate() ) {
    QList<QDate> l;
    l.append( toDate() );
    return l;
  }
  else
    return value<QList<QDate> >();
}


QList<QTime> Nepomuk::KMetaData::Variant::toTimeList() const
{
  if( isTime() ) {
    QList<QTime> l;
    l.append( toTime() );
    return l;
  }
  else
    return value<QList<QTime> >();
}


QList<QDateTime> Nepomuk::KMetaData::Variant::toDateTimeList() const
{
  if( isDateTime() ) {
    QList<QDateTime> l;
    l.append( toDateTime() );
    return l;
  }
  else
    return value<QList<QDateTime> >();
}


QList<QUrl> Nepomuk::KMetaData::Variant::toUrlList() const
{
  if( isUrl() ) {
    QList<QUrl> l;
    l.append( toUrl() );
    return l;
  }
  else
    return value<QList<QUrl> >();
}


QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::Variant::toResourceList() const
{
  if( isResource() ) {
    QList<Resource> l;
    l.append( toResource() );
    return l;
  }
  else
    return value<QList<Resource> >();
}


bool Nepomuk::KMetaData::Variant::isList() const
{
  return( isIntList() ||
	  isBoolList() ||
	  isDoubleList() ||
	  isStringList() ||
	  isDateList() ||
	  isTimeList() ||
	  isDateTimeList() ||
	  isUrlList() ||
	  isResourceList() );
}


int Nepomuk::KMetaData::Variant::type() const
{
  return QVariant::userType();
}


int Nepomuk::KMetaData::Variant::simpleType() const
{
  if( isIntList() )
    return qMetaTypeId<int>();
  else if( isBoolList() )
    return qMetaTypeId<bool>();
  else if( isDoubleList() )
    return qMetaTypeId<double>();
  else if( isStringList() )
    return qMetaTypeId<QString>();
  else if( isDateList() )
    return qMetaTypeId<QDate>();
  else if( isTimeList() )
    return qMetaTypeId<QTime>();
  else if( isDateTimeList() )
    return qMetaTypeId<QDateTime>();
  else if( isUrlList() )
    return qMetaTypeId<QUrl>();
  else if( isResourceList() )
    return qMetaTypeId<Resource>();
  else
    return QVariant::userType();
}


bool Nepomuk::KMetaData::Variant::operator==( const Variant& other ) const
{
  return QVariant::operator==( other );
}


bool Nepomuk::KMetaData::Variant::operator!=( const Variant& other ) const
{
  return QVariant::operator!=( other );
}
