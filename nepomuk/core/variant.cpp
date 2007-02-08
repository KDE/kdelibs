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

#include <kdebug.h>


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


Nepomuk::KMetaData::Variant::Variant( qlonglong i )
  : QVariant( i )
{
}


Nepomuk::KMetaData::Variant::Variant( uint i )
  : QVariant( i )
{
}


Nepomuk::KMetaData::Variant::Variant( qulonglong i )
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


Nepomuk::KMetaData::Variant::Variant( const char* string )
  : QVariant( string )
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


Nepomuk::KMetaData::Variant::Variant( const QList<qlonglong>& i )
  : QVariant()
{
  setValue( i );
}


Nepomuk::KMetaData::Variant::Variant( const QList<uint>& i )
  : QVariant()
{
  setValue( i );
}


Nepomuk::KMetaData::Variant::Variant( const QList<qulonglong>& i )
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


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( qlonglong i )
{
  setValue( i );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( uint i )
{
  setValue( i );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( qulonglong i )
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


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<qlonglong>& i )
{
  setValue( i );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<uint>& i )
{
  setValue( i );
  return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<qulonglong>& i )
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


void Nepomuk::KMetaData::Variant::append( qlonglong i )
{
  QList<qlonglong> l = toInt64List();
  l.append( i );
  operator=( l );
}


void Nepomuk::KMetaData::Variant::append( uint i )
{
  QList<uint> l = toUnsignedIntList();
  l.append( i );
  operator=( l );
}


void Nepomuk::KMetaData::Variant::append( qulonglong i )
{
  QList<qulonglong> l = toUnsignedInt64List();
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
  if( v.simpleType() == QVariant::Int ) {
    operator=( toIntList() += v.toIntList() );
  }
  else if( v.simpleType() == QVariant::UInt ) {
    operator=( toUnsignedIntList() += v.toUnsignedIntList() );
  }
  else if( v.simpleType() == QVariant::LongLong ) {
    operator=( toInt64List() += v.toInt64List() );
  }
  else if( v.simpleType() == QVariant::ULongLong ) {
    operator=( toUnsignedInt64List() += v.toUnsignedInt64List() );
  }
  else if( v.simpleType() == QVariant::Bool ) {
    operator=( toBoolList() += v.toBoolList() );
  }
  else if( v.simpleType() == QVariant::Double ) {
    operator=( toDoubleList() += v.toDoubleList() );
  }
  else if( v.simpleType() == QVariant::String ) {
    operator=( toStringList() += v.toStringList() );
  }
  else if( v.simpleType() == QVariant::Date ) {
    operator=( toDateList() += v.toDateList() );
  }
  else if( v.simpleType() == QVariant::Time ) {
    operator=( toTimeList() += v.toTimeList() );
  }
  else if( v.simpleType() == QVariant::DateTime ) {
    operator=( toDateTimeList() += v.toDateTimeList() );
  }
  else if( v.simpleType() == QVariant::Url ) {
    operator=( toUrlList() += v.toUrlList() );
  }
  else if( v.simpleType() == qMetaTypeId<Resource>() ) {
    operator=( toResourceList() += v.toResourceList() );
  }
  else
    kDebug(300004) << "(Variant::append) unknown type: " << v.simpleType() << endl;
}


bool Nepomuk::KMetaData::Variant::isInt() const
{
  return( type() == QVariant::Int );
}


bool Nepomuk::KMetaData::Variant::isInt64() const
{
  return( type() == QVariant::LongLong );
}


bool Nepomuk::KMetaData::Variant::isUnsignedInt() const
{
  return( type() == QVariant::UInt );
}


bool Nepomuk::KMetaData::Variant::isUnsignedInt64() const
{
  return( type() == QVariant::ULongLong );
}


bool Nepomuk::KMetaData::Variant::isBool() const
{
  return( type() == QVariant::Bool );
}


bool Nepomuk::KMetaData::Variant::isDouble() const
{
  return( type() == QVariant::Double );
}


bool Nepomuk::KMetaData::Variant::isString() const
{
  return( type() == QVariant::String );
}


bool Nepomuk::KMetaData::Variant::isDate() const
{
  return( type() == QVariant::Date );
}


bool Nepomuk::KMetaData::Variant::isTime() const
{
  return( type() == QVariant::Time );
}


bool Nepomuk::KMetaData::Variant::isDateTime() const
{
  return( type() == QVariant::DateTime );
}


bool Nepomuk::KMetaData::Variant::isUrl() const
{
  return( type() == QVariant::Url );
}


bool Nepomuk::KMetaData::Variant::isResource() const
{
  return( type() == qMetaTypeId<Resource>() );
}


bool Nepomuk::KMetaData::Variant::isIntList() const
{
  return( type() == qMetaTypeId<QList<int> >() );
}


bool Nepomuk::KMetaData::Variant::isUnsignedIntList() const
{
  return( type() == qMetaTypeId<QList<uint> >() );
}


bool Nepomuk::KMetaData::Variant::isInt64List() const
{
  return( type() == qMetaTypeId<QList<qlonglong> >() );
}


bool Nepomuk::KMetaData::Variant::isUnsignedInt64List() const
{
  return( type() == qMetaTypeId<QList<qulonglong> >() );
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
  return( type() == QVariant::StringList );
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


qlonglong Nepomuk::KMetaData::Variant::toInt64() const
{
  return value<qlonglong>();
}


uint Nepomuk::KMetaData::Variant::toUnsignedInt() const
{
  return value<uint>();
}


qulonglong Nepomuk::KMetaData::Variant::toUnsignedInt64() const
{
  return value<qulonglong>();
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
  kDebug(300004) << "(Variant::toString() converting... " << QMetaType::typeName(type()) << endl;
  if( isList() )
    return toStringList().join( "," );

  else if( isInt() )
    return QString::number( toInt() );
  else if( isInt64() )
    return QString::number( toInt64() );
  else if( isUnsignedInt() )
    return QString::number( toUnsignedInt() );
  else if( isUnsignedInt64() )
    return QString::number( toUnsignedInt64() );
  else if( isBool() )
    return ( toBool() ? QString("true") : QString("false" ) );
  else if( isDouble() )
    return QString::number( toDouble() );
  else if( isDate() )
    return toDate().toString();
  else if( isTime() )
    return toTime().toString();
  else if( isDateTime() )
    return toDateTime().toString();
  else if( isUrl() )
    return toUrl().toString();
  else if( isResource() )
    return toResource().uri();
  else
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
  return listValue<int>();
}


QList<qlonglong> Nepomuk::KMetaData::Variant::toInt64List() const
{
  return listValue<qlonglong>();
}


QList<uint> Nepomuk::KMetaData::Variant::toUnsignedIntList() const
{
  return listValue<uint>();
}


QList<qulonglong> Nepomuk::KMetaData::Variant::toUnsignedInt64List() const
{
  return listValue<qulonglong>();
}


QList<bool> Nepomuk::KMetaData::Variant::toBoolList() const
{
  return listValue<bool>();
}


QList<double> Nepomuk::KMetaData::Variant::toDoubleList() const
{
  return listValue<double>();
}


template<typename T> QStringList convertToStringList( const QList<T>& l )
{
  QStringList sl;
  QListIterator<T> it( l );
  while( it.hasNext() )
    sl.append( Nepomuk::KMetaData::Variant( it.next() ).toString() );
//   for( QList<T>::const_iterator it = l.constBegin(); it != l.constEnd(); ++it )
//     sl.append( Nepomuk::KMetaData::Variant( *it ).toString() );
  return sl;
}

QStringList Nepomuk::KMetaData::Variant::toStringList() const
{
  //  kDebug(300004) << "(Variant::toStringList() converting... " << QMetaType::typeName(simpleType()) << endl;
  if( !isList() )
    return QStringList( toString() );

  else if( isIntList() )
    return convertToStringList<int>( toIntList() );
  else if( isInt64List() )
    return convertToStringList<qlonglong>( toInt64List() );
  else if( isUnsignedIntList() )
    return convertToStringList<uint>( toUnsignedIntList() );
  else if( isUnsignedInt64List() )
    return convertToStringList<qulonglong>( toUnsignedInt64List() );
  else if( isBoolList() )
    return convertToStringList<bool>( toBoolList() );
  else if( isDoubleList() )
    return convertToStringList<double>( toDoubleList() );
  else if( isDateList() )
    return convertToStringList<QDate>( toDateList() );
  else if( isTimeList() )
    return convertToStringList<QTime>( toTimeList() );
  else if( isDateTimeList() )
    return convertToStringList<QDateTime>( toDateTimeList() );
  else if( isUrlList() )
    return convertToStringList<QUrl>( toUrlList() );
  else if( isResourceList() )
    return convertToStringList<Resource>( toResourceList() );
  else
    return value<QStringList>();
}


QList<QDate> Nepomuk::KMetaData::Variant::toDateList() const
{
  return listValue<QDate>();
}


QList<QTime> Nepomuk::KMetaData::Variant::toTimeList() const
{
  return listValue<QTime>();
}


QList<QDateTime> Nepomuk::KMetaData::Variant::toDateTimeList() const
{
  return listValue<QDateTime>();
}


QList<QUrl> Nepomuk::KMetaData::Variant::toUrlList() const
{
  return listValue<QUrl>();
}


QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::Variant::toResourceList() const
{
  return listValue<Resource>();
}


bool Nepomuk::KMetaData::Variant::isList() const
{
  return( isIntList() ||
	  isInt64List() ||
	  isUnsignedIntList() ||
	  isUnsignedInt64List() ||
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
    return QVariant::Int;
  else if( isInt64List() )
    return QVariant::LongLong;
  else if( isUnsignedIntList() )
    return QVariant::UInt;
  else if( isUnsignedInt64List() )
    return QVariant::ULongLong;
  else if( isBoolList() )
    return QVariant::Bool;
  else if( isDoubleList() )
    return QVariant::Double;
  else if( isStringList() )
    return QVariant::String;
  else if( isDateList() )
    return QVariant::Date;
  else if( isTimeList() )
    return QVariant::Time;
  else if( isDateTimeList() )
    return QVariant::DateTime;
  else if( isUrlList() )
    return QVariant::Url;
  else if( isResourceList() )
    return qMetaTypeId<Resource>();
  else
    return QVariant::userType();
}


bool Nepomuk::KMetaData::Variant::operator==( const Variant& other ) const
{
  if( other.type() != this->type() )
    return false;

  else if( isInt() )
    return other.toInt() == toInt();
  else if( isInt64() )
    return other.toInt64() == toInt64();
  else if( isUnsignedInt() )
    return other.toUnsignedInt() == toUnsignedInt();
  else if( isUnsignedInt64() )
    return other.toUnsignedInt64() == toUnsignedInt64();
  else if( isBool() )
    return other.toBool() == toBool();
  else if( isDouble() )
    return other.toDouble() == toDouble();
  else if( isDate() )
    return other.toDate() == toDate();
  else if( isTime() )
    return other.toTime() == toTime();
  else if( isDateTime() )
    return other.toDateTime() == toDateTime();
  else if( isUrl() )
    return other.toUrl() == toUrl();
  else if( isResource() )
    return other.toResource() == toResource();
  else if( isString() )
    return other.value<QString>() == value<QString>();
  else if( isIntList() )
    return other.toIntList() == toIntList();
  else if( isBoolList() )
    return other.toBoolList() == toBoolList();
  else if( isDoubleList() )
    return other.toDoubleList() == toDoubleList();
  else if( isStringList() )
    return other.value<QStringList>() == value<QStringList>();
  else if( isDateList() )
    return other.toDateList() == toDateList();
  else if( isTimeList() )
    return other.toTimeList() == toTimeList();
  else if( isDateTimeList() )
    return other.toDateTimeList() == toDateTimeList();
  else if( isUrlList() )
    return other.toUrlList() == toUrlList();
  else if( isResourceList() )
    return other.toResourceList() == toResourceList();
  else
    return QVariant::operator==( other );
}


bool Nepomuk::KMetaData::Variant::operator!=( const Variant& other ) const
{
  return !operator==( other );
}
