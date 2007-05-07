/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "variant.h"
#include "generated/resource.h"

#include <soprano/literalvalue.h>

#include <kdebug.h>

#include <QtCore/QVariant>



class Nepomuk::KMetaData::Variant::Private
{
public:
    QVariant value;
};


Nepomuk::KMetaData::Variant::Variant()
    : d( new Private )
{
}


Nepomuk::KMetaData::Variant::~Variant()
{
    delete d;
}


Nepomuk::KMetaData::Variant::Variant( const Variant& other )
    : d( new Private )
{
    operator=( other );
}


Nepomuk::KMetaData::Variant::Variant( const QVariant& other )
    : d( new Private )
{
    if ( other.userType() == QVariant::Int ||
         other.userType() == QVariant::LongLong ||
         other.userType() == QVariant::UInt ||
         other.userType() == QVariant::ULongLong ||
         other.userType() == QVariant::Bool ||
         other.userType() == QVariant::Double ||
         other.userType() == QVariant::String ||
         other.userType() == QVariant::Date ||
         other.userType() == QVariant::Time ||
         other.userType() == QVariant::DateTime ||
         other.userType() == QVariant::Url ||
         other.userType() == qMetaTypeId<Resource>() ||
         other.userType() == qMetaTypeId<QList<int> >() ||
         other.userType() == qMetaTypeId<QList<qlonglong> >() ||
         other.userType() == qMetaTypeId<QList<uint> >() ||
         other.userType() == qMetaTypeId<QList<qulonglong> >() ||
         other.userType() == qMetaTypeId<QList<bool> >() ||
         other.userType() == qMetaTypeId<QList<double> >() ||
         other.userType() == QVariant::StringList ||
         other.userType() == qMetaTypeId<QList<QDate> >() ||
         other.userType() == qMetaTypeId<QList<QTime> >() ||
         other.userType() == qMetaTypeId<QList<QDateTime> >() ||
         other.userType() == qMetaTypeId<QList<QUrl> >() ||
         other.userType() == qMetaTypeId<QList<Resource> >() ) {
        d->value = other;
    }
}


Nepomuk::KMetaData::Variant::Variant( int i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::KMetaData::Variant::Variant( qlonglong i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::KMetaData::Variant::Variant( uint i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::KMetaData::Variant::Variant( qulonglong i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::KMetaData::Variant::Variant( bool b )
    : d( new Private )
{
    d->value.setValue( b );
}


Nepomuk::KMetaData::Variant::Variant( double v )
    : d( new Private )
{
    d->value.setValue( v );
}


Nepomuk::KMetaData::Variant::Variant( const char* string )
    : d( new Private )
{
    d->value.setValue( QString::fromLatin1(string) );
}


Nepomuk::KMetaData::Variant::Variant( const QString& string )
    : d( new Private )
{
    d->value.setValue( string );
}


Nepomuk::KMetaData::Variant::Variant( const QDate& date )
    : d( new Private )
{
    d->value.setValue( date );
}


Nepomuk::KMetaData::Variant::Variant( const QTime& time )
    : d( new Private )
{
    d->value.setValue( time );
}


Nepomuk::KMetaData::Variant::Variant( const QDateTime& datetime )
    : d( new Private )
{
    d->value.setValue( datetime );
}


Nepomuk::KMetaData::Variant::Variant( const QUrl& url )
    : d( new Private )
{
    d->value.setValue( url );
}


Nepomuk::KMetaData::Variant::Variant( const Nepomuk::KMetaData::Resource& r )
    : d( new Private )
{
    d->value.setValue( r );
}


Nepomuk::KMetaData::Variant::Variant( const QList<int>& i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::KMetaData::Variant::Variant( const QList<qlonglong>& i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::KMetaData::Variant::Variant( const QList<uint>& i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::KMetaData::Variant::Variant( const QList<qulonglong>& i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::KMetaData::Variant::Variant( const QList<bool>& b )
    : d( new Private )
{
    d->value.setValue( b );
}


Nepomuk::KMetaData::Variant::Variant( const QList<double>& v )
    : d( new Private )
{
    d->value.setValue( v );
}


Nepomuk::KMetaData::Variant::Variant( const QStringList& stringlist )
    : d( new Private )
{
    d->value.setValue( stringlist );
}


Nepomuk::KMetaData::Variant::Variant( const QList<QDate>& date )
    : d( new Private )
{
    d->value.setValue( date );
}


Nepomuk::KMetaData::Variant::Variant( const QList<QTime>& time )
    : d( new Private )
{
    d->value.setValue( time );
}


Nepomuk::KMetaData::Variant::Variant( const QList<QDateTime>& datetime )
    : d( new Private )
{
    d->value.setValue( datetime );
}


Nepomuk::KMetaData::Variant::Variant( const QList<QUrl>& url )
    : d( new Private )
{
    d->value.setValue( url );
}



Nepomuk::KMetaData::Variant::Variant( const QList<Resource>& r )
    : d( new Private )
{
    d->value.setValue( r );
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const Variant& v )
{
    d->value = v.d->value;
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( int i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( qlonglong i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( uint i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( qulonglong i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( bool b )
{
    d->value.setValue( b );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( double v )
{
    d->value.setValue( v );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QString& string )
{
    d->value.setValue( string );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QDate& date )
{
    d->value.setValue( date );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QTime& time )
{
    d->value.setValue( time );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QDateTime& datetime )
{
    d->value.setValue( datetime );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QUrl& url )
{
    d->value.setValue( url );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const Resource& r )
{
    d->value.setValue( r );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<int>& i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<qlonglong>& i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<uint>& i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<qulonglong>& i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<bool>& b )
{
    d->value.setValue( b );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<double>& v )
{
    d->value.setValue( v );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QStringList& stringlist )
{
    d->value.setValue( stringlist );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<QDate>& date )
{
    d->value.setValue( date );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<QTime>& time )
{
    d->value.setValue( time );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<QDateTime>& datetime )
{
    d->value.setValue( datetime );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<QUrl>& url )
{
    d->value.setValue( url );
    return *this;
}


Nepomuk::KMetaData::Variant& Nepomuk::KMetaData::Variant::operator=( const QList<Resource>& r )
{
    d->value.setValue( r );
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
    return d->value.value<int>();
}


qlonglong Nepomuk::KMetaData::Variant::toInt64() const
{
    return d->value.value<qlonglong>();
}


uint Nepomuk::KMetaData::Variant::toUnsignedInt() const
{
    return d->value.value<uint>();
}


qulonglong Nepomuk::KMetaData::Variant::toUnsignedInt64() const
{
    return d->value.value<qulonglong>();
}


bool Nepomuk::KMetaData::Variant::toBool() const
{
    return d->value.value<bool>();
}


double Nepomuk::KMetaData::Variant::toDouble() const
{
    return d->value.value<double>();
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
    else if( isDouble() ) // FIXME: decide on a proper double encoding or check if there is one in xml schema
        return QString::number( toDouble(), 'e', 10 );
    // FIXME: use the correct data and time encoding of XML Schema
    else if( isDate() )
        return Soprano::LiteralValue( toDate() ).toString();
    else if( isTime() )
        return Soprano::LiteralValue( toTime() ).toString();
    else if( isDateTime() )
        return Soprano::LiteralValue( toDateTime() ).toString();
    else if( isUrl() )
        return toUrl().toString();
    else if( isResource() ) {
        Resource r = toResource();
        if( !r.uri().isEmpty() )
            return r.uri();
        else
            return r.identifiers().first();
    }
    else
        return d->value.value<QString>();
}


QDate Nepomuk::KMetaData::Variant::toDate() const
{
    return d->value.value<QDate>();
}


QTime Nepomuk::KMetaData::Variant::toTime() const
{
    return d->value.value<QTime>();
}


QDateTime Nepomuk::KMetaData::Variant::toDateTime() const
{
    return d->value.value<QDateTime>();
}


QUrl Nepomuk::KMetaData::Variant::toUrl() const
{
    return d->value.value<QUrl>();
}


Nepomuk::KMetaData::Resource Nepomuk::KMetaData::Variant::toResource() const
{
    return d->value.value<Resource>();
}



QList<int> Nepomuk::KMetaData::Variant::toIntList() const
{
    if( isInt() ) {
        QList<int> l;
        l.append( toInt() );
        return l;
    }
    else
        return d->value.value<QList<int> >();
}


QList<qlonglong> Nepomuk::KMetaData::Variant::toInt64List() const
{
    if( isInt64() ) {
        QList<qlonglong> l;
        l.append( toInt64() );
        return l;
    }
    else
        return d->value.value<QList<qlonglong> >();
}


QList<uint> Nepomuk::KMetaData::Variant::toUnsignedIntList() const
{
    if( isUnsignedInt() ) {
        QList<uint> l;
        l.append( toUnsignedInt() );
        return l;
    }
    else
        return d->value.value<QList<uint> >();
}


QList<qulonglong> Nepomuk::KMetaData::Variant::toUnsignedInt64List() const
{
    if( isUnsignedInt64() ) {
        QList<qulonglong> l;
        l.append( toUnsignedInt64() );
        return l;
    }
    else
        return d->value.value<QList<qulonglong> >();
}


QList<bool> Nepomuk::KMetaData::Variant::toBoolList() const
{
    if( isBool() ) {
        QList<bool> l;
        l.append( toBool() );
        return l;
    }
    else
        return d->value.value<QList<bool> >();
}


QList<double> Nepomuk::KMetaData::Variant::toDoubleList() const
{
    if( isDouble() ) {
        QList<double> l;
        l.append( toDouble() );
        return l;
    }
    else
        return d->value.value<QList<double> >();
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
    if( !d->value.isValid() )
        return QStringList();

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
        return d->value.value<QStringList>();
}


QList<QDate> Nepomuk::KMetaData::Variant::toDateList() const
{
    if( isDate() ) {
        QList<QDate> l;
        l.append( toDate() );
        return l;
    }
    else
        return d->value.value<QList<QDate> >();
}


QList<QTime> Nepomuk::KMetaData::Variant::toTimeList() const
{
    if( isTime() ) {
        QList<QTime> l;
        l.append( toTime() );
        return l;
    }
    else
        return d->value.value<QList<QTime> >();
}


QList<QDateTime> Nepomuk::KMetaData::Variant::toDateTimeList() const
{
    if( isDateTime() ) {
        QList<QDateTime> l;
        l.append( toDateTime() );
        return l;
    }
    else
        return d->value.value<QList<QDateTime> >();
}


QList<QUrl> Nepomuk::KMetaData::Variant::toUrlList() const
{
    if( isUrl() ) {
        QList<QUrl> l;
        l.append( toUrl() );
        return l;
    }
    else
        return d->value.value<QList<QUrl> >();
}


QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::Variant::toResourceList() const
{
    if( isResource() ) {
        QList<Resource> l;
        l.append( toResource() );
        return l;
    }
    else
        return d->value.value<QList<Resource> >();
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
    return d->value.userType();
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
        return d->value.userType();
}


Nepomuk::KMetaData::Variant Nepomuk::KMetaData::Variant::fromString( const QString& value, int type )
{
    // first check the types that are not supported by Soprano since they are not literal types
    if( type == qMetaTypeId<Resource>() ) {
        return Variant( Resource( value ) );
    }
    else if ( type == int( QVariant::Url ) ) {
        return Variant( QUrl( value ) );
    }

    // let Soprano do the rest
    else {
        return Variant( Soprano::LiteralValue::fromString( value, ( QVariant::Type )type ).variant() );
    }
}


bool Nepomuk::KMetaData::Variant::operator==( const Variant& other ) const
{
    if( other.simpleType() != this->simpleType() )
        return false;

    if( isInt() || isIntList() )
        return other.toIntList() == toIntList();
    else if( isInt64() || isInt64List() )
        return other.toInt64List() == toInt64List();
    else if( isUnsignedInt() || isUnsignedIntList() )
        return other.toUnsignedIntList() == toUnsignedIntList();
    else if( isUnsignedInt64() || isUnsignedInt64List() )
        return other.toUnsignedInt64List() == toUnsignedInt64List();
    else if( isBool() || isBoolList() )
        return other.toBoolList() == toBoolList();
    else if( isDouble() || isDoubleList() )
        return other.toDoubleList() == toDoubleList();
    else if( isString() || isStringList() )
        return other.d->value.value<QStringList>() == d->value.value<QStringList>();
    else if( isDate() || isDateList() )
        return other.toDateList() == toDateList();
    else if( isTime() || isTimeList() )
        return other.toTimeList() == toTimeList();
    else if( isDateTime() || isDateTimeList() )
        return other.toDateTimeList() == toDateTimeList();
    else if( isUrl() || isUrlList() )
        return other.toUrlList() == toUrlList();
    else if( isResource() || isResourceList() )
        return other.toResourceList() == toResourceList();
    else
        return ( d->value == other.d->value );
}


bool Nepomuk::KMetaData::Variant::operator!=( const Variant& other ) const
{
    return !operator==( other );
}


QVariant Nepomuk::KMetaData::Variant::variant() const
{
    return d->value;
}


QDebug operator<<( QDebug dbg, const Nepomuk::KMetaData::Variant& v )
{
    if( v.isList() )
        dbg << v.toStringList();
    else
        dbg << v.toString();
    return dbg;
}
